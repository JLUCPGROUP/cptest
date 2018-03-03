/*
 * XMLBuilder.cpp
 *
 *  Created on: 2016年6月21日
 *      Author: leezear
 */

#include "XBuilder.h"

namespace cp {

XBuilder::XBuilder(const string file_name, const XmlReaderType type) {
	const bool res = initial(file_name);
	if (res&& type == XRT_BM_PATH) {
		benchmark_path_ = GetBMFile();
		del();
		initial(benchmark_path_);
	}
	else if (res&& type == XRT_BM) {
		benchmark_path_ = file_name;
	}
	else {
		cout << "error" << endl;
	}
}

XBuilder::~XBuilder() {
	if (root_) {
		delete parser_;
		XMLPlatformUtils::Terminate();
	}
}

bool XBuilder::initial(const string s) {
	if (s == "") {
		return false;
	}

	try {
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException & toCatch) {
		// Do your failure processing here
		return false;
	}
	// Do your actual work with Xerces-C++ here.
	parser_ = new XercesDOMParser();
	parser_->setValidationScheme(XercesDOMParser::Val_Always);
	parser_->setDoNamespaces(true);

	parser_->parse(s.c_str());
	document_ = parser_->getDocument();
	root_ = document_->getDocumentElement();

	if (!root_) {
		delete (parser_);
		parser_ = nullptr;
		return false;
	}
	return true;
}

string XBuilder::GetBMFile() const {
	DOMNodeList *bmfile_list = root_->getElementsByTagName(XMLString::transcode("BMFile"));
	DOMNode *bmfile = bmfile_list->item(0);
	char* bm_name = XMLString::transcode(bmfile->getFirstChild()->getNodeValue());
	string bm_file(bm_name);
	XMLString::release(&bm_name);
	return bm_file;
}

//u32 XBuilder::getConstraintsCount() {
//	DOMNode *cons_node = root_->getElementsByTagName(XMLString::transcode("constraints"))->item(0);
//	const int num_cons = XMLString::parseInt(cons_node->getAttributes()->getNamedItem(XMLString::transcode("nbConstraints"))->getTextContent());
//	return num_cons;
//}

void XBuilder::generateDomains(XModel *model) const {
	DOMNode *doms_nodes = root_->getElementsByTagName(XMLString::transcode("domains"))->item(0);
	const int num_doms = XMLString::parseInt(
		doms_nodes->getAttributes()->getNamedItem(XMLString::transcode("nbDomains"))->getTextContent());
	DOMNodeList *dom_nodes = root_->getElementsByTagName(XMLString::transcode("domain"));

	for (int i = 0; i < num_doms; ++i) {
		DOMNode * node = dom_nodes->item(i);
		const int size = XMLString::parseInt(
			node->getAttributes()->getNamedItem(XMLString::transcode("nbValues"))->getTextContent());
		char* values = XMLString::transcode(node->getFirstChild()->getNodeValue());
		model->add(i, size, values);
		XMLString::release(&values);
	}
}

void XBuilder::generateVariables(XModel *model) const {
	DOMNode *vars_node = root_->getElementsByTagName(XMLString::transcode("variables"))->item(0);
	const int num_vars = XMLString::parseInt(
		vars_node->getAttributes()->getNamedItem(XMLString::transcode("nbVariables"))->getTextContent());
	DOMNodeList* var_nodes = root_->getElementsByTagName(XMLString::transcode("variable"));

	for (u32 i = 0; i < num_vars; ++i) {
		DOMNode * node = var_nodes->item(i);
		char * dom_id_str = XMLString::transcode(
			node->getAttributes()->getNamedItem(XMLString::transcode("domain"))->getTextContent());
		model->add(i, dom_id_str);
		XMLString::release(&dom_id_str);
	}
}

void XBuilder::generateRelations(XModel* model) const {
	DOMNode* relations_node = root_->getElementsByTagName(XMLString::transcode("relations"))->item(0);
	const int relations_count = XMLString::parseInt(
		relations_node->getAttributes()->getNamedItem(XMLString::transcode("nbRelations"))->getTextContent());
	DOMNodeList* relation_nodes = root_->getElementsByTagName(XMLString::transcode("relation"));

	for (int i = 0; i < relations_count; ++i) {
		DOMNode *node = relation_nodes->item(i);
		const int arity = XMLString::parseInt(
			node->getAttributes()->getNamedItem(XMLString::transcode("arity"))->getTextContent());
		char* semantics = XMLString::transcode(
			node->getAttributes()->getNamedItem(XMLString::transcode("semantics"))->getTextContent());
		const int size = XMLString::parseInt(
			node->getAttributes()->getNamedItem(XMLString::transcode("nbTuples"))->getTextContent());
		if (size != 0) {
			char* ts_str = XMLString::transcode(node->getFirstChild()->getNodeValue());
			model->add(i, arity, size, semantics, ts_str);
			XMLString::release(&ts_str);
		}
		else {
			model->add(i, arity, size, semantics, "");
		}
		XMLString::release(&semantics);

	}
}

void XBuilder::generateConstraints(XModel* model) const {
	DOMNode* cons_node = root_->getElementsByTagName(XMLString::transcode("constraints"))->item(0);
	const int num_cons = XMLString::parseInt(
		cons_node->getAttributes()->getNamedItem(XMLString::transcode("nbConstraints"))->getTextContent());
	DOMNodeList* con_nodes = root_->getElementsByTagName(XMLString::transcode("constraint"));

	for (int i = 0; i < num_cons; ++i) {
		DOMNode * node = con_nodes->item(i);
		const int arity = XMLString::parseInt(
			node->getAttributes()->getNamedItem(XMLString::transcode("arity"))->getTextContent());
		char *scp_str = XMLString::transcode(
			node->getAttributes()->getNamedItem(XMLString::transcode("scope"))->getTextContent());
		char * rel_id = XMLString::transcode(
			node->getAttributes()->getNamedItem(XMLString::transcode("reference"))->getTextContent());
		model->add(i, rel_id, arity, scp_str);
	}
}

void XBuilder::del() const {
	if (root_) {
		delete parser_;
		XMLPlatformUtils::Terminate();
	}
}

void XBuilder::GenerateXModelFromXml(XModel *model) {
	xm_ = model;
	generateDomains(model);
	generateVariables(model);
	generateRelations(model);
	generateConstraints(model);
}

void XBuilder::GenerateHModel(HModel* hm) {
	xm_ = new XModel();
	GenerateXModelFromXml(xm_);
	for (auto& v : xm_->vars)
		hm->AddVar("", xm_->doms[v->xdom_id]->vals);
	for (auto c : xm_->cons)
		hm->AddTab(xm_->rels[c->rel_id]->semantics, xm_->rels[c->rel_id]->tuples, c->scope);
	delete xm_;
}

string XBuilder::path() const {
	return  benchmark_path_;
}
string XBuilder::file_name() const {
	return file_name_;
}
}
