#include "../../include/debugviz/flow_graph.h"
#include <fstream>
#include <vector>
#include <memory>
#include <string>

struct node
{
	std::string name;
	std::vector<std::string> inputs, outputs;

	node(const std::string& n, const std::vector<std::string>& i, const std::vector<std::string>& o) :
		name(n), inputs(i), outputs(o) {}
};
struct connection
{
	size_t out, out_slot, in, in_slot;
};
struct connectivity
{
	struct connection_view
	{
		const std::string& out;
		const std::string& out_slot;
		const std::string& in;
		const std::string& in_slot;
	};
	struct iterator
	{
		const connectivity& parent;
		size_t index;
		iterator(const connectivity& p, size_t i) : parent(p), index(i) {}
		connection_view operator*() const { return parent[index]; }
		bool operator!=(const iterator& it) const { return index != it.index; }
		iterator& operator++() { index++; return *this; }
	};

	connectivity(const std::vector<node>& n, std::vector<connection> c) : connections(std::move(c)), nodes(n) {}
	iterator begin() const { return iterator(*this, 0); }
	iterator end() const { return iterator(*this, connections.size()); }
	connection_view operator[](size_t i) const
	{
		const connection& c = connections[i];
		return { nodes[c.out].name, nodes[c.out].outputs[c.out_slot], nodes[c.in].name, nodes[c.in].inputs[c.in_slot] };
	}

private:
	std::vector<connection> connections;
	const std::vector<node>& nodes;
};

int main()
{
	const std::vector<node> nodes =
	{
		{ "plop",  {},                   { "val" } },
		{ "modif", { "vv", "uu", "yy" }, { "value" } },
		{ "cst",   {},                   { "v" } },
		{ "add",   { "x", "y" },         { "value", "u", "v", "w" } },
		{ "final", { "w" },              {} }
	};
	const connectivity connections =
	{
		nodes,
		{
			{ 0, 0, 1, 1 },
			{ 1, 0, 3, 1 },
			{ 2, 0, 3, 0 },
			{ 3, 0, 4, 0 }
		}
	};

	//*
	std::ofstream("test2.html") << debugviz::flow_graph("Test", nodes, connections);
	/*/
	std::ofstream test_file("test.html");
	debugviz::write_flow_graph(test_file, "Test", nodes, connections);
	//*/
}
