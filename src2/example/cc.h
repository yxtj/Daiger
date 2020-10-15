#pragma once
#include "common/def.h"
#include "api/api.h"
#include <string>

struct ConnectedComponent
	: public AppKernel
{
	using value_t = id_t;
	using neighbor_t = id_t;
	// typedef std::vector<neighbor_t> neighbor_list_t;
	// typedef Node<id_t, value_t> node_t;
	
	static const std::string name;

	struct MyOperation : public OperationMax<value_t, neighbor_t> {
		using typename Operation<value_t, neighbor_t>::value_t;
		using typename Operation<value_t, neighbor_t>::neighbor_t;
		using typename Operation<value_t, neighbor_t>::neighbor_list_t;
		using typename Operation<value_t, neighbor_t>::node_t;

		virtual void init(const std::vector<std::string>& arg_line);

		virtual node_t preprocess_node(const id_t& k, neighbor_list_t& neighbors);
		virtual value_t func(const node_t& n, const neighbor_t& neighbor);
		virtual priority_t priority(const node_t& n);
	};

	class MySeparator : public ArgumentSeparator {
	public:
		virtual AppArguments separate(const std::vector<std::string>& args);
	};

	typedef MySeparator separator_t;
	typedef MyOperation operation_t;
	typedef IOHandlerUnweighted<value_t> iohandler_t;
	typedef ProgressorValue<value_t, neighbor_t> progressor_t;
	typedef GlobalHolder<value_t, neighbor_t> graph_t;

	virtual std::string getName() const;

	virtual void reg();

	virtual ArgumentSeparator* generateSeparator();
	virtual OperationBase* generateOperation();
	virtual IOHandlerBase* generateIOHandler();
	virtual ProgressorBase* generateProgressor();
	virtual GlobalHolderBase* generateGraph();
	virtual PrioritizerBase* generatePrioritizer(const std::string& name);

};

