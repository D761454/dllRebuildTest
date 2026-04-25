#include <stdlib.h>
#include <iostream>
#include <Windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#include "CameraComponent.h"

// on build, engine and game mains will need access to engine dll headers to get funcs on build
// presumably will only need access to the engine update loop to call it as everything will ideally be handled by engine e.g. load 1st scene / start scene, then enter main loop
// most game functionality handled by user scripts on game objects
// will likely need to store editor in same dir / close to engine project for easy include and rebuilds
#include "../testDLL/interface.h"
#include "Watcher.h"

#include <chrono>

struct ReloadDLL {
	std::wstring PathToDLL;
	std::vector<std::string> ProcsToLoad;

	HMODULE Module = NULL;
	uint64_t LastWrite = 0;
	std::map<std::string, void*> Procs;
};

//https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw
// To run a batch file, you must start the command interpreter; set lpApplicationName to cmd.exe
// and set lpCommandLine to the following arguments: /c plus the name of the batch file.
// ideally ran on a thread as will block program until done
static bool RebuildBatch(const std::wstring& batchPath) {
	// verify path
	DWORD attrs = GetFileAttributesW(batchPath.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES) {
		return false;
	}

	// get system path
	wchar_t systemDir[MAX_PATH] = { 0 };
	if (GetSystemDirectoryW(systemDir, MAX_PATH) == 0) {
		return false;
	}
	std::wstring cmdExe = std::wstring(systemDir) + L"\\cmd.exe";

	// create the command line
	std::wstring commandLine = L"/C \"" + batchPath + L"\"";
	std::vector<wchar_t> cmdlineBuf(commandLine.begin(), commandLine.end());
	cmdlineBuf.push_back(L'\0');

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	BOOL created = CreateProcessW(
		cmdExe.c_str(), // needed for batch scripts
		cmdlineBuf.data(), // stuff to execute
		NULL, NULL,
		FALSE,
		CREATE_NO_WINDOW, // prevent opening console window
		NULL, NULL,
		&si, &pi
	);

	if (!created) {
		DWORD err = GetLastError();
		return false;
	}

	// wait forever until process is terminated
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exitCode = 0;
	if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
		exitCode = static_cast<DWORD>(-1);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return exitCode == 0;
}

static void EraseNDoubleSlashes(std::wstring& dir, int n) {
	for (int i = n; i--;) {
		dir.erase(dir.find_last_of(L'\\'));
	}
	dir.erase(dir.find_last_of(L'\\') + 1);
}

// can add an update ptr function in here to auto update all method ptrs
static void PollDLLChanges(ReloadDLL& rcpp) {
	// open
	HANDLE hDLLFile = CreateFileW(rcpp.PathToDLL.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDLLFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Check for changes
	uint64_t lastWriteTime;
	if (GetFileTime(hDLLFile, NULL, NULL, (FILETIME*)&lastWriteTime) &&
		rcpp.LastWrite < lastWriteTime)
	{
		// Remove all references to the old module
		if (rcpp.Module)
		{
			FreeLibrary((HMODULE)rcpp.Module);
			rcpp.Module = NULL;
			rcpp.Procs.clear();
		}

		// Copy the DLL
		std::wstring tmpname = rcpp.PathToDLL + L".rcpp.dll";
		if (CopyFileW(rcpp.PathToDLL.c_str(), tmpname.c_str(), FALSE))
		{
			// Load the DLL and get functions
			rcpp.Module = LoadLibraryW(tmpname.c_str());
			if (rcpp.Module)
			{
				for (const auto& p : rcpp.ProcsToLoad)
				{
					rcpp.Procs[p] = GetProcAddress((HMODULE)rcpp.Module, p.c_str());
				}
				rcpp.LastWrite = lastWriteTime;
			}
		}
	}

	CloseHandle(hDLLFile);
}

static bool PollDirChanges(std::wstring dir, ReloadDLL* rlcpp = nullptr, bool* pauser = nullptr) {
	// https://learn.microsoft.com/en-us/windows/win32/fileio/obtaining-directory-change-notifications
	if (pauser) {
		*pauser = false;
	}

	HANDLE dchanges;
	dchanges = FindFirstChangeNotificationW(dir.c_str(), true, FILE_NOTIFY_CHANGE_LAST_WRITE);

	if (dchanges == INVALID_HANDLE_VALUE)
	{
		CloseHandle(dchanges);
		return false;
	}

	DWORD dwWaitStatus = WaitForSingleObject(dchanges, INFINITE);

	bool reBuilt = false;

	switch (dwWaitStatus) {
	case WAIT_TIMEOUT:
		break;
	case WAIT_OBJECT_0:

		if (pauser) {
			*pauser = true;
		}

		reBuilt = RebuildBatch(dir + L"\\ReBuildDLL.bat");

		if (!reBuilt) {
			std::cout << "MSBuild failed or returned non-zero exit code!\n";
			break;
		}
		else {
			std::cout << "ReBuilt\n";

			if (rlcpp) {
				PollDLLChanges(*rlcpp);
			}

			if (pauser) {
				*pauser = false;
			}

			break;
		}
	default:
		break;
	}

	CloseHandle(dchanges);
	return reBuilt;
}

int main(int argc, char** argv)
{
	REGISTER_FACTORY(CameraFactory, Components::CameraComponent);

	auto start = std::chrono::system_clock::now();

	ReloadDLL rcpp;

	std::wstring directory(argv[0], argv[0] + strlen(argv[0]));
	directory.erase(directory.find_last_of(L'\\') + 1);

	rcpp.PathToDLL = directory + L"testDLL.dll";
	rcpp.ProcsToLoad = { "TestFunc", "Register" };

	PollDLLChanges(rcpp);

	bool running = true;
	bool paused = false;

	auto reload_ptr = (decltype(TestFunc)*)rcpp.Procs["TestFunc"];
	auto reg_ptr = (decltype(Register)*)rcpp.Procs["Register"];

	EraseNDoubleSlashes(directory, 3);

	GetWatcherThread()->SetWJob(std::bind(&PollDirChanges, directory + L"testDLL", &rcpp, &paused));

	int i = 0;
	int max = 20;

	float dt = 0.0f;

	World::Objects::Object* child = World::Objects::SceneRootObject::GetInstance().CreateChild("ChildObj");

	std::cout << child->m_name << std::endl;

	child->AddComponent<Components::CameraComponent>();

	if (reg_ptr) {
		reg_ptr(&World::Objects::SceneRootObject::GetInstance());
	}

	World::Objects::SceneRootObject::GetInstance().m_componentFactories.Create("testcomponent", child);

	while (running) {
		auto b = std::chrono::system_clock::now();

		if (!paused) {
			if (reload_ptr) {
				reload_ptr();
			}

			i = 0;

			auto e = std::chrono::system_clock::now();
			dt = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(e - b).count();
		}
		else {
			// loading bar ""
			system("cls");
			for (int j = i; j--;) {
				std::cout << "#";
			}
			for (int j = i; j < max; j++) {
				std::cout << "-";
			}
			std::cout << std::endl;
			i++;
			if (i > max) {
				i = 0;
			}
			int sleep = (1500 * dt);
			std::cout << dt << std::endl;
			std::cout << sleep << std::endl;

			Sleep(sleep);
			reload_ptr = (decltype(TestFunc)*)rcpp.Procs["TestFunc"];
		}

		auto now = std::chrono::system_clock::now();

		if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 60.0f) {
			running = false;

			// on program end, may need to add a txt file to be modified to trigger the thread and have a bool ptr to make the rebuild its final task before ending,
			// just clean as obv can just press x to close program
		}
	}

	return 0;
}