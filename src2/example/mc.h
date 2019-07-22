#pragma once
#include "common/def.h"
#include "api/api.h"
#include <string>

struct MarkovChain
	: public AppKernel
{
	using value_t = double;
	using neighbor_t = std::pair<id_t, double>;
	using node_t = Node<double, neighbor_t>;
	
	static const std::string name;

	struct MyOperation : public OperationAddition<value_t, neighbor_t> {
		using typename Operation<value_t, neighbor_t>::value_t;
		using typename Operation<value_t, neighbor_t>::neighbor_t;
		using typename Operation<value_t, neighbor_t>::neighbor_list_t;
		using typename Operation<value_t, neighbor_t>::node_t;

		virtual void init(const std::vector<std::string>& arg_line, const size_t nInstance);

		virtual node_t preprocess_node(const id_t& k, neighbor_list_t& neighbors);
		virtual value_t func(const node_t& n, const neighbor_t& neighbor);
		virtual priority_t priority(const node_t& n);
	private:
		bool use_degree;

		struct InnerHelper;
		InnerHelper* impl;
	};

	class MySeparator : public ArgumentSeparator {
	public:
		virtual AppArguments separate(const std::vector<std::string>& args);
	};

	typedef MySeparator separator_t;
	typedef MyOperation operation_t;
	typedef IOHandlerWeighted<value_t, neighbor_t> iohandler_t;
	typedef ProgressorSquare<value_t, neighbor_t> progressor_t;
	typedef GlobalHolder<value_t, neighbor_t> graph_t;

	virtual std::string getName() const;

	virtual void reg();

	virtual ArgumentSeparator* generateSeparator();
	virtual OperationBase* generateOperation();
	virtual IOHandlerBase* generateIOHandler();
	virtual ProgressorBase* generateProgressor();
	virtual GlobalHolderBase* generateGraph();
};

