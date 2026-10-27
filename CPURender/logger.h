#pragma once


struct Logger {

	template<typename ... Ts>
	static void Log(const char* toFormat, Ts... args) {
		
		if(shouldLog)
			printf(toFormat, args...);
	}

	static void LogThisFrame()
	{
		shouldLog = true;
	}

	static void endFrame() { shouldLog = false; };

protected:
	static bool shouldLog;
};