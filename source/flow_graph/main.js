const fs = require("fs");
const minimize_js = require("uglify-es").minify;
const minimize_html = require('html-minifier').minify;

// Assemble and minimize scripts
// ------------------------------------------------------------------------------------------------
var scripts = ["layout.js", "collisions.js", "render.js"]
var assembled_script = "";
for(var sc of scripts)
	assembled_script += fs.readFileSync(sc) + "\n\n";
const min = minimize_js(assembled_script, {
	compress:
	{
		unsafe: true,
		unsafe_math: true,
		inline: true,
		passes: 2
	},
	output:
	{
		beautify: false,
		quote_style: 1 // Always use single quote (=> no need to escape in C++ string)
	}
});
if(min.error) console.log(min.error);
const min_script = min.code;

// Assemble and minimize html
// ------------------------------------------------------------------------------------------------
var html = "" + fs.readFileSync("flow_graph.html");
html = html.replace("%SCRIPT%", min_script);
var min_html = minimize_html(html, {
	minifyCSS: true,
	collapseWhitespace: true,
	removeComments: true,
	removeRedundantAttributes: true,
	quoteCharacter: "'"
});

// Assemble cpp
// ------------------------------------------------------------------------------------------------
var cpp_html = min_html.replace("%CPP_TITLE%", '" << title << "')
var parts = cpp_html.split("%CPP_STRUCTURE%");
var cpp_html1 = "\t\tstream << " + [parts[0].slice(0, 79)]
	.concat(parts[0].slice(79).match(/[\s\S]{1,85}/g))
	.filter(x => x)
	.map(l => '"' + l + '"')
	.join("\n\t\t\t");
var cpp_html2 = ';\n\t\tstructure();\n\t\tstream << ' + [parts[1].slice(0, 79)]
	.concat(parts[1].slice(79).match(/[\s\S]{1,85}/g))
	.filter(x => x)
	.map(l => '"' + l + '"')
	.join("\n\t\t\t");
cpp_html = cpp_html1 + cpp_html2 + ";";
var cpp = "" + fs.readFileSync("flow_graph.h");
cpp = "// WARNING: auto-generated, do not modify !\n\n" + cpp.replace("%STREAM_PACKED_HTML%", cpp_html);
cpp = cpp.split("\r").join("");
fs.writeFile("../../include/debugviz/flow_graph.h", cpp);
