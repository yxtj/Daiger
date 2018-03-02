#pragma once
#include <string>
#include <map>
#include <functional>

/*
Template class of simple factory which works with string options.
*/
template <class PRDCT>
class FactoryTemplate
{
protected:
	using createFun = std::function<PRDCT*()>;
	static std::map<std::string, createFun> contGen;
	static std::map<std::string, std::string> contUsage;
public:
	// used in setting program option
	// specialization class should give defination of these two values
	static const std::string optName;
	static const std::string usagePrefix;

	FactoryTemplate() = delete;

	template <class T>
	static void registerClass(const std::string& name);
	template <class T>
	static void registerClass(const std::string& name, const std::string& usage);

	static std::string getUsage();

	static bool isValid(const std::string& name);

	static PRDCT* generate(const std::string& name);
};


template <class PRDCT>
template <class T>
inline void FactoryTemplate<PRDCT>::registerClass(const std::string& name) {
	contGen[name] = []() {
		return new T();
	};
}

template <class PRDCT>
template <class T>
inline void FactoryTemplate<PRDCT>::registerClass(const std::string & name, const std::string & usage) {
	registerClass<T>(name);
	contUsage[name] = usage;
}

template<class PRDCT>
std::string FactoryTemplate<PRDCT>::getUsage()
{
	std::string res = usagePrefix;
	int cnt = 0;
	for(const auto& usg : contUsage) {
		res += "Option " + std::to_string(++cnt) + ": " + usg.first
			+ "\n" + usg.second + "\n";
	}
	return res;
}

template<class PRDCT>
inline bool FactoryTemplate<PRDCT>::isValid(const std::string & name) {
	return contGen.find(name) != contGen.end();
}

template<class PRDCT>
inline PRDCT* FactoryTemplate<PRDCT>::generate(const std::string & name)
{
	PRDCT* res = nullptr;
	if(isValid(name)) {
		res = contGen.at(name)();
	}
	return res;
}
