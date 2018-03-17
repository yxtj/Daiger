#pragma once
#include "common/def.h"
#include "api/api.h"
#include <string>

struct ShortestPath
	: public AppKernel
{
	using value_t = double;
	using neighbor_t = std::pair<id_t, double>;
	
	static const std::string name;

	struct MyOperation : public OperationMin<value_t, neighbor_t> {
		using typename Operation<value_t, neighbor_t>::value_t;
		using typename Operation<value_t, neighbor_t>::neighbor_t;
		using typename Operation<value_t, neighbor_t>::neighbor_list_t;
		using typename Operation<value_t, neighbor_t>::node_t;

		virtual void init(const std::vector<std::string>& arg_line);

		virtual value_t init_value(const id_t& k, const neighbor_list_t& neighbors);
		virtual value_t func(const node_t& n, const neighbor_t& neighbor);
		virtual priority_t priority(const node_t& n);
	private:
		id_t source;
		bool use_degree;
	};

	class MySeparator : public ArgumentSeparator {
	public:
		virtual AppArguments separate(const std::vector<std::string>& args);
	};

	typedef MySeparator separator_t;
	typedef MyOperation operation_t;
	typedef IOHandlerWeighted<value_t, neighbor_t> iohandler_t;
	typedef TerminatorDiff<value_t, neighbor_t> terminator_t;
	typedef GlobalHolder<value_t, neighbor_t> graph_t;

	virtual std::string getName() const;

	virtual void reg();

	virtual ArgumentSeparator* generateSeparator();
	virtual OperationBase* generateOperation();
	virtual IOHandlerBase* generateIOHandler();
	virtual TerminatorBase* generateTerminator();
	virtual GlobalHolderBase* generateGraph();
};

