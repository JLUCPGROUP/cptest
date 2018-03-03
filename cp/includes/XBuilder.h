/*
 * XBuilder.h
 *
 *  Created on: 2016年6月21日
 *      Author: leezear
 */

#ifndef XMLBUILDER_H_
#define XMLBUILDER_H_

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMTreeWalker.hpp>
#include <string>
#include "XModel.h"
#include "HModel.h"

using namespace xercesc_3_1;

namespace cp {
/**
*\brief some Types of bankmark file type
*/
enum BMFileType {
	BFT_TXT,			///<txt doc
	BFT_XCSP_2_0_INT,	///<xcsp v2.0 intension
	BFT_XCSP_2_1_INT,	///<xcsp v2.1 intension
	BFT_XCSP_3_0_INT,	///<xcsp v3.0 intension
	BFT_XCSP_2_0_EXT,	///<xcsp v2.0 extension
	BFT_XCSP_2_1_EXT,	///<xcsp v2.1 extension
	BFT_XCSP_3_0_EXT	///<xcsp v3.0 extension
};

enum XmlReaderType {
	XRT_BM_PATH,		///<banchmark path file
	XRT_BM				///<banchmark
};

/**
 * \brief XBuilder
 * Generate constraint network model for xml benchmark file
 * */
class XBuilder {
public:

	/**
	 * \brief Constructors and initialization
	 * \param[in] i_file_name	The name of file.
	 * \param[in] i_type		The type of file, path file or benchmark file.
	 */
	XBuilder(string file_name, const XmlReaderType type);
	virtual ~XBuilder();

	/**
	 * \brief Constructors and initialization
	 * \param[out] network	Pointer of model
	 * \return model generate result
	 *	-<em>false</em> fail
	 *	-<em>true</em> succeed
	 */
	void GenerateXModelFromXml(XModel * xm);
	void GenerateHModel(HModel* hm);

	//GenerateXModelFromXml
	string path() const;
	string file_name() const;
	/**
	 * \brief GetBMFile
	 * \return Banchmark file path
	 */
	std::string GetBMFile() const;

private:
	//NetworkFeatures feature_;		///<Feature of Network
	XModel* xm_ = nullptr;				///<Constarint Network pointer
	string benchmark_path_;			///<XML file name(path)
	string file_name_;
	XmlReaderType type_;			///<file type
	XercesDOMParser *parser_;
	DOMElement *root_;
	xercesc_3_1::DOMDocument *document_;

	/**
	 * \brief createDomain
	 * \return Created domain
	 */
	 //	void createDomain(XMLModel * network, const u32 dom_id, const u32 dom_size,
	 //			std::string domain_values);
	//std::vector<int> generateValues(const u32 dom_size, string values_str);

	void generateDomains(XModel *model) const;

	void generateVariables(XModel *model) const;

	void generateRelations(XModel *model) const;

	//void generateTabulars(XModel *model);

	//void generatePredicates(XModel *model);

	void generateConstraints(XModel *model) const;


	//u32 getConstraintsCount();

	//RelationType getRelationTpye();

	//void getNetworkFeature(XMLModel * network);

	//int getMaxArity();
	void del() const;
	bool initial(const string s);

};


}/* namespace cudacsp */

#endif /* XMLBUILDER_H_ */
