#pragma once
#include "common/def.h"
#include "api/api.h"
#include <string>

struct ConnectedComponent
	: public AppKernel
{
	typedef id_t value_t;
	typedef id_t neighbor_t;
	// typedef std::vector<neighbor_t> neighbor_list_t;
	// typedef Node<id_t, value_t> node_t;
	
	static const std::string name;

	struct MyOperation : public Operation<value_t, neighbor_t> {
		using typename Operation<value_t, neighbor_t>::value_t;
		using typename Operation<value_t, neighbor_t>::neighbor_t;
		using typename Operation<value_t, neighbor_t>::neighbor_list_t;
		using typename Operation<value_t, neighbor_t>::node_t;

		virtual void init(const std::vector<std::string>& arg_line);

		virtual value_t init_value(const id_t& k, const neighbor_list_t& neighbors);

		virtual value_t identity_element() const;
		virtual value_t oplus(const value_t& a, const value_t& b);
		virtual value_t func(const node_t& n, const neighbor_t& neighbor);

		virtual bool is_selective(){ return true; }
		virtual bool better(const value_t& a, const value_t& b);
		
		virtual priority_t priority(const node_t& n);
	};

	class Separator : public ArgumentSeparator {
	public:
		virtual AppArguments separate(const std::vector<std::string>& args);
	};

	typedef Separator separator_t;
	typedef MyOperation operation_t;
	typedef IOHandlerUnweighted<value_t> iohandler_t;
	typedef TerminatorStop<value_t, neighbor_t> terminator_t;
	typedef GlobalHolder<value_t, neighbor_t> graph_t;

	virtual std::string getName() const;

	virtual void reg();

	virtual ArgumentSeparator* generateSeparator();
	virtual OperationBase* generateOperation();
	virtual IOHandlerBase* generateIOHandler();
	virtual TerminatorBase* generateTerminator();

};

