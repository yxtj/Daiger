#pragma once
#include "common/def.h"
#include "api/api.h"
#include <string>

struct ConnectedComponent {
	typedef key_t value_t;
	typedef key_t neighbor_t;
	// typedef std::vector<neighbor_t> neighbor_list_t;
	// typedef Node<key_t, value_t> node_t;
	
	static const std::string name;
	
	 // regist operation, io-handler, terminator, arg-separator into factories
	ConnectedComponent();

	struct Operation : public Operation<value_t, neighbor_t> {
		virtual void init(const std::vector<std::string>& arg_line);

		virtual value_t init_value(const key_t& k, const neighbor_list_t& neighbors);

		virtual value_t identity_element() const;
		virtual value_t oplus(value_t& a, const value_t& b);
		virtual value_t func(const node_t& n, const neighbor_t& neighbor);

		virtual bool is_selective(){ return true; }
		virtual bool better(const value_t& a, const value_t& b);
		
		virtual priority_t priority(const node_t& n);
	};

	class Separator : public ArgumentSeparator {
	public:
		virtual AppArguments separate(const std::vector<std::string>& args);
	};

	typedef Operation operation_t;
	typedef IOHandlerUnweighted<value_t> iohandler_t;
	typedef TerminatorStop<value_t, neighbor_t> terminator_t;
	typedef Separator separator_t;

};

