#pragma once
#include "common/def.h"
#include "api/api.h"
#include <string>

struct Katz
	: public AppKernel
{
	using value_t = double;
	using neighbor_t = id_t;
	using node_t = Node<value_t, neighbor_t>;
	
	static const std::string name;

	struct MyOperation : public OperationAddition<value_t, neighbor_t> {
		using typename Operation<value_t, neighbor_t>::value_t;
		using typename Operation<value_t, neighbor_t>::neighbor_t;
		using typename Operation<value_t, neighbor_t>::neighbor_list_t;
		using typename Operation<value_t, neighbor_t>::node_t;

		virtual void init(const std::vector<std::string>& arg_line, const size_t nInstance);

		virtual node_t preprocess_node(const id_t& k, neighbor_list_t& neighbors);
		virtual std::vector<DummyNode> dummy_nodes();
		virtual bool is_dummy_node(const id_t& id);
		virtual value_t func(const node_t& n, const neighbor_t& neighbor);
		virtual priority_t priority(const node_t& n);
	private:
		id_t source;
		double beta;
		bool use_degree;
		id_t dummy_id;
	};

	class MySeparator : public ArgumentSeparator {
	public:
		virtual AppArguments separate(const std::vector<std::string>& args);
	};
	class MyTerminator : public TerminatorDiff<value_t, neighbor_t> {
	public:
		virtual double progress(const Node<value_t, neighbor_t>& n){ return helper_progress_vsquare(n); };
	};

	typedef MySeparator separator_t;
	typedef MyOperation operation_t;
	typedef IOHandlerUnweighted<value_t> iohandler_t;
	typedef MyTerminator terminator_t;
	typedef GlobalHolder<value_t, neighbor_t> graph_t;

	virtual std::string getName() const;

	virtual void reg();

	virtual ArgumentSeparator* generateSeparator();
	virtual OperationBase* generateOperation();
	virtual IOHandlerBase* generateIOHandler();
	virtual TerminatorBase* generateTerminator();
	virtual GlobalHolderBase* generateGraph();
};
