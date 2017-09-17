// BSD 3-Clause Licence //////////////////////////////////////////////////////////////////////// //
// Copyright (c) 2017 Thibault Lescoat, All rights reserved.                                     //
//                                                                                               //
// Redistribution and use in source and binary forms, with or without modification, are          //
// permitted provided that the following conditions are met:                                     //
//                                                                                               //
// * Redistributions of source code must retain the above copyright notice, this list of         //
//   conditions and the following disclaimer.                                                    //
//                                                                                               //
// * Redistributions in binary form must reproduce the above copyright notice, this list of      //
//   conditions and the following disclaimer in the documentation and/or other materials         //
//   provided with the distribution.                                                             //
//                                                                                               //
// * Neither the name of the copyright holder nor the names of its contributors may be used to   //
//   endorse or promote products derived from this software without specific prior written       //
//   permission.                                                                                 //
//                                                                                               //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   //
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    //
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE //
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    //
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     //
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF          //
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                    //
// ///////////////////////////////////////////////////////////////////////////////////////////// //
#pragma once
#include <iostream>
#include <type_traits>

namespace debugviz
{
namespace detail
{
	template<typename T> struct to_void { using type = void; };
	template<typename T> using void_t = typename to_void<T>::type;
	template<typename T> using no_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

	template<typename, typename T, typename = void_t<T>>
	struct is_streamable : std::false_type {};
	template<typename S, typename T>
	struct is_streamable<S, T, void_t<decltype(std::declval<S&>() << std::declval<no_cvref<T>>())>>
		: std::true_type {};

	template<class S, class T> using streamable_name     = is_streamable<S, decltype(no_cvref<T>::name)>;
	template<class S, class T> using streamable_out      = is_streamable<S, decltype(no_cvref<T>::out)>;
	template<class S, class T> using streamable_out_slot = is_streamable<S, decltype(no_cvref<T>::out_slot)>;
	template<class S, class T> using streamable_in       = is_streamable<S, decltype(no_cvref<T>::in)>;
	template<class S, class T> using streamable_in_slot  = is_streamable<S, decltype(no_cvref<T>::in_slot)>;
}

	/// Outputs a html page to visualize a flow graph
	/** This function serialize a flow graph into a stream, as an html page that can be viewed on a
		standard web browser (it does not need Internet connectivity, as all needed scripts/CSS are
		fully included in the html). Please bear in mind that no html escaping will be done!

		A flow graph is a graph where nodes possess *input* and *output* ports (or slots), which
		are used as endpoints for the edges of the graph. Connections can only go from an output
		slot to an input slot, and can be viewed as "data comes out of a specific node at some port
		and is fed into the specified input of another node".

		In the following by 'range' we mean a type that can be iterated using a
		range-based for loop, and by 'streamable' we mean a type that can be serialized into the
		given stream.

		\note for std::ostream, there exist a wrapper so you can stream a flow graph as usual:
		`stream << bdgviz::flow_graph(title, nodes, connections);`.

		\param stream The stream in which to serialize the flow graph html visualization
		\param title Title of the html page (must be streamable)
		\param nodes Nodes of the graph. This structure must be a range, and elements must have:
			- a 'name' field, streamable
			- an 'inputs' field, range of streamables
			- an 'outputs' field, range of streamables
		\param connections Links between nodes in the graph. This must be a range, and its elements
			must have four streamable fields: 'out', 'out_slot', 'in', 'in_slot'
		\return The given stream
	*/
	template<typename S, typename T, typename N, typename C>
	S& write_flow_graph(S& stream, const T& title, const N& nodes, const C& connections)
	{
		auto structure = [&]()
		{
			stream << "nodes: [";
			for(const auto& n : nodes)
			{
				static_assert(detail::streamable_name<S, decltype(n)>::value,
					"Nodes must have a 'name' field that supports 'stream << node.name'");

				stream << "{ name: \"" << n.name << "\", inputs: [";
				for(const auto& s : n.inputs)
				{
					static_assert(detail::is_streamable<S, decltype(s)>::value,
						"Node inputs slots must support 'stream << slot'");
					stream << "\"" << s << "\",";
				}
				stream << "], outputs: [";
				for(const auto& s : n.outputs)
				{
					static_assert(detail::is_streamable<S, decltype(s)>::value,
						"Node outputs slots must support 'stream << slot'");
					stream << "\"" << s << "\",";
				}
				stream << "] },";
			}
			stream << "], connections: [";
			for(const auto& c : connections)
			{
				static_assert(detail::streamable_out<S, decltype(c)>::value,
					"Connections must have a 'out' field that supports 'stream << connection.out'");
				static_assert(detail::streamable_out_slot<S, decltype(c)>::value,
					"Connections must have a 'out_slot' field that supports 'stream << connection.out_slot'");
				static_assert(detail::streamable_in<S, decltype(c)>::value,
					"Connections must have a 'in' field that supports 'stream << connection.in'");
				static_assert(detail::streamable_in_slot<S, decltype(c)>::value,
					"Connections must have a 'in_slot' field that supports 'stream << connection.in_slot'");

				stream << "{ out: \"" << c.out << "\", out_slot: \"" << c.out_slot
					<< "\", in: \"" << c.in << "\", in_slot: \"" << c.in_slot << "\" },";
			}
			stream << "]";
		};

%STREAM_PACKED_HTML%

		return stream;
	}

namespace detail
{
	template<typename T, typename N, typename C>
	struct flow_graph_data
	{
		const T& title;
		const N& nodes;
		const C& connections;
	};

	template<typename T, typename N, typename C>
	std::ostream& operator<<(std::ostream& os, const flow_graph_data<T, N, C>& g)
	{
		return write_flow_graph<std::ostream, T, N, C>(os, g.title, g.nodes, g.connections);
	}
}

	/// Convenience function for use with standard output streams
	/** A wrapper on write_flow_graph for std::ostream allowing constructs like:
		`std::cout << debugviz::flow_graph("Some title", nodes, connections);`.
		\see write_flow_graph for a more complete explanation of the parameters. */
	template<typename T, typename N, typename C>
	detail::flow_graph_data<T, N, C> flow_graph(const T& title, const N& nodes, const C& connections)
	{
		return { title, nodes, connections };
	}
}
