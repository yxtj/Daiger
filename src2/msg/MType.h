#pragma once

typedef int msg_t;

struct MType {
	// Basic Control
	static constexpr int CReply = 0;
	static constexpr int COnline = 1;
	static constexpr int CRegister = 2;
	static constexpr int CWorkers = 3;
	static constexpr int CClear = 4;
	static constexpr int CShutdown = 7;
	static constexpr int CTerminate = 8;
	static constexpr int CAlive = 9;

	// Procedure Control
	static constexpr int CProcedure = 10;
	static constexpr int CFinish = 11;

	// Graph Loading and related
	static constexpr int GNode = 20;
	static constexpr int GValue = 21;
	static constexpr int GDelta = 22;
	static constexpr int GINCache = 22;

	// Value Update 
	static constexpr int VUpdate = 30;
	static constexpr int VRequest = 31;
	static constexpr int VReply = 32;

	// Process and Progress (Termination)
	static constexpr int PApply = 40;
	static constexpr int PSend = 41;
	static constexpr int PReport = 42;
	static constexpr int PRequest = 43;

	// Staticstics
	static constexpr int SGather = 60;
};