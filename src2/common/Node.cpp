#include "Node.h"

using namespace std;

Node::Node(const key_t& k, const value_t& v, const value_t& u):
	NodeBasic(k, v), u(u)
{}
