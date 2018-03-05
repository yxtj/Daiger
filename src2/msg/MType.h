#pragma once

typedef int msg_t;

struct MType {
	// Basic Control
	static constexpr int CReply = 0;
	static constexpr int CRegister = 1;
	static constexpr int CReady = 2;
	static constexpr int CEnd = 3;
	static constexpr int CEndForce = 4;
	static constexpr int CAlive = 9;

	// Procedure Control
	static constexpr int PLoadGraph = 10;
	static constexpr int PLoadValue = 11;
	static constexpr int PLoadDelta = 12;
	static constexpr int PUpdate = 13;
	static constexpr int POutput = 14;

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