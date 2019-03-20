#!/usr/bin/env bash

rm clang.res >> /dev/null
DIR_SPEC="/home/sheep/Minf_project/rose-source-code-moderniser/benchmarks/SPEC_Benchmarks_srcs"
FLAGS="-I${DIR_SPEC}/523.xalancbmk_r/src -I${DIR_SPEC}/510.parest_r/src/include -I${DIR_SPEC}/510.parest_r/src/"
echo "FLAGS: $FLAGS"
CONFIG="{CheckOptions: [{key: modernize-loop-convert.MinConfidence, value: 'risky'}]}"
for i in $@; do
	x=$(clang-tidy -checks='modernize-loop-convert' $i  -config="{CheckOptions: [{key: modernize-loop-convert.MinConfidence, value: 'risky'}]}" -- $FLAGS -std=c++11 \
		| grep "warning: use range-based for loop instead" | wc -l)
	echo -e "Name: $i,\thits: $x" >> clang.res
done 

# 523.xalancbmk_r
#TraverseSchema.cpp XMLChar.cpp XalanXMLChar.cpp XPath.cpp SGXMLScanner.cpp DTDScanner.cpp XPathProcessorImpl.cpp XSLTEngineImpl.cpp IGXMLScanner.cpp DGXMLScanner.cpp IGXMLScanner2.cpp StylesheetExecutionContextDefault.cpp XMLUri.cpp XTemplateSerializer.cpp XMLScanner.cpp SchemaValidator.cpp ElemNumber.cpp WFXMLScanner.cpp 

# 531.deepsjeng_r./


# 541.leela_r
# FastBoard.cpp GTP.cpp FastState.cpp UCTSearch.cpp UCTNode.cpp Matcher.cpp SGFTree.cpp
