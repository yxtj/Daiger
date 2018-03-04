#pragma once
#include "common/def.h"
#include "api/api.h"
#include <string>

struct ConnectedComponent {
	typedef key_t value_t;
	typedef key_t neighbor_t;
	
	static const std::string name{"cc"};

	struct Operation : public Operation<value_t, neighbor_t> {
		virtual bool parse(const std::vector<std::string>& arg_line);

		virtual value_t init_value(const key_t& k, const neighbor_list_t& neighbors);

		virtual value_t identity_element() const;
		virtual value_t oplus(value_t& a, const value_t& b);
		virtual value_t func(const node_t& n, const neighbor_t& neighbor);

		virtual bool is_selective(){ return true; }
		virtual bool better(const value_t& a, const value_t& b);
		
		virtual priority_t priority(const node_t& n);
	};

	class Separator : public ArgumentSeparator {
		virtual AppArguments separate(const std::vector<std::string>& args);
	}

	typedef Operation operation_t;
	typedef IOHandler<value_t, neighbor_t> iohandler_t;
	typedef TerminatorStop<value_t, neighbor_t> terminator_t;
	typedef Separator separator_t;

	operation_t opr;
	iohandler_t ioh;
	terminator_t tmt;
	separator_t sep;

};

