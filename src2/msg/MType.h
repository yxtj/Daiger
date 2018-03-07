#pragma once

typedef int msg_t;

struct MType {
	// Basic Control
	static constexpr int CReply = 0;
	static constexpr int CRegister = 1;
	static constexpr int CWorkers = 2;
	static constexpr int CClear = 3;
	static constexpr int CShutdown = 4;
	static constexpr int CTerminate = 8;
	static constexpr int CAlive = 9;

	// Procedure Control
	static constexpr int CProcedure = 10;
	static constexpr int CFinish = 11;

	// Graph Loading and related
	static constexpr int LGraph = 20;
	static constexpr int LValue = 21;
	static constexpr int LDelta = 22;

	// Value Update 
	static constexpr int VUpdate = 30;
	static constexpr int VRequest = 31;
	static constexpr int VReply = 32;

	// Progress and Termination
	static constexpr int TReport = 40;
	static constexpr int TRequest = 41;

	// Staticstics
	static constexpr int SGather = 60;
};