#include "ScriptingEngineNet.h"

#include "src/Core/Logger.h"
#include "src/Core/FileSystem.h"


#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/NativeArray.hpp>
#include <Coral/Attribute.hpp>


void ExceptionCallback(std::string_view InMessage)
{
	const std::string message = std::string("Unhandled native exception: ") + std::string(InMessage);
	Nuake::Logger::Log(message, ".net", Nuake::CRITICAL);
}

namespace Nuake
{
	ScriptingEngineNet& ScriptingEngineNet::Get()
	{
		static ScriptingEngineNet instance;
		return instance;
	}

	void Log(Coral::NativeString string)
	{
		Logger::Log(string.ToString(), ".net", VERBOSE);
	}

	void ScriptingEngineNet::Initialize()
	{
		auto coralDir = "";
		Coral::HostSettings settings =
		{
			.CoralDirectory = coralDir,
			.ExceptionCallback = ExceptionCallback
		};
		Coral::HostInstance hostInstance;
		hostInstance.Initialize(settings);

		auto loadContext = hostInstance.CreateAssemblyLoadContext("NuakeEngineContext");
		auto& assembly = loadContext.LoadAssembly("NuakeNet.dll");

		assembly.AddInternalCall("Nuake.Net.Engine", "LoggerLogIcall", reinterpret_cast<void*>(&Log));
		assembly.UploadInternalCalls();

		auto& engineType = assembly.GetType("Nuake.Net.Engine");
		auto engineInstance = engineType.CreateInstance();
		engineInstance.InvokeMethod("Log");
	}
}