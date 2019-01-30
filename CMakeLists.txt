cmake_minimum_required(VERSION 3.8)
project(ml2sql,ml2sqlserver)


set(CMAKE_CXX_STANDARD 14)


add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/mllib/Ml_Lib.h
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/Build-mllib-Header.py ${CMAKE_CURRENT_SOURCE_DIR}/resources/mllib
        DEPENDS resources/mllib/*
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating code for the ml libs."
)



# Use ML.g4
add_custom_command(
        OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLBaseVisitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLBaseVisitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLLexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLLexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLVisitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML/MLVisitor.h
        COMMAND java -jar /usr/local/lib/antlr-4.7.1-complete.jar -Dlanguage=Cpp -no-listener -visitor -o ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/ML ML.g4
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/grammar/
        DEPENDS resources/grammar/ML.g4
        COMMENT "Generating MlParser, MlLexer, and MlVisitor."
)


# Use Pre.g4
add_custom_command(
        OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreBaseVisitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreBaseVisitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreLexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreLexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreVisitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre/PreVisitor.h
        COMMAND java -jar /usr/local/lib/antlr-4.7.1-complete.jar -Dlanguage=Cpp -no-listener -visitor -o ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/Pre Pre.g4
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/grammar/
        DEPENDS resources/grammar/Pre.g4
        COMMENT "Generating  PreParser, PreLexer, and PreVisitor."
)


# Use tf.g4
add_custom_command(
        OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfBaseVisitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfBaseVisitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfLexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfLexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfVisitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf/tfVisitor.h
        COMMAND java -jar /usr/local/lib/antlr-4.7.1-complete.jar -Dlanguage=Cpp -no-listener -visitor -o ${CMAKE_CURRENT_SOURCE_DIR}/antlr4/tf tf.g4
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/grammar/
        DEPENDS resources/grammar/tf.g4
        COMMENT "Generating tfParser, tfLexer, and tfVisitor."
)



include_directories(include)
include_directories(antlr4)
include_directories(/usr/local/include/antlr4-runtime/)


add_executable(ml2sql
        include/codeGenerator/python/tensorFlow/Graph.h
        include/codeGenerator/python/tensorFlow/MytfVisitor.h
        include/codeGenerator/python/tensorFlow/TFGraphBuilder.h
        include/codeGenerator/python/PythonGenerator.h
        include/codeGenerator/sql/HyperCode.h
        include/codeGenerator/sql/HyperGenerator.h
        include/codeGenerator/sql/PostgresCode.h
        include/codeGenerator/sql/PostgresGenerator.h
        include/codeGenerator/sql/SqlGenerator.h
        include/codeGenerator/sql/SqlOverrideFunctions.h
        include/codeGenerator/Generator.h
        include/helper/Helpers.h
        include/variable/Accessor.h
        include/variable/Interval.h
        include/variable/Variable.h
        include/visitor/MyMLVisitor.h
        include/visitor/MyPreVisitor.h
        antlr4/ML/MLBaseVisitor.cpp
        antlr4/ML/MLBaseVisitor.h
        antlr4/ML/MLLexer.cpp
        antlr4/ML/MLLexer.h
        antlr4/ML/MLParser.cpp
        antlr4/ML/MLParser.h
        antlr4/ML/MLVisitor.cpp
        antlr4/ML/MLVisitor.h
        antlr4/Pre/PreBaseVisitor.cpp
        antlr4/Pre/PreBaseVisitor.h
        antlr4/Pre/PreLexer.cpp
        antlr4/Pre/PreLexer.h
        antlr4/Pre/PreParser.cpp
        antlr4/Pre/PreParser.h
        antlr4/Pre/PreVisitor.cpp
        antlr4/Pre/PreVisitor.h
        antlr4/tf/tfBaseVisitor.cpp
        antlr4/tf/tfBaseVisitor.h
        antlr4/tf/tfLexer.cpp
        antlr4/tf/tfLexer.h
        antlr4/tf/tfParser.cpp
        antlr4/tf/tfParser.h
        antlr4/tf/tfVisitor.cpp
        antlr4/tf/tfVisitor.h
        src/codeGenerator/python/tensorFlow/Graph.cpp
        src/codeGenerator/python/tensorFlow/MytfVisitor.cpp
        src/codeGenerator/python/tensorFlow/TFGraphBuilder.cpp
        src/codeGenerator/python/PythonGenerator.cpp
        src/codeGenerator/sql/HyperGenerator.cpp
        src/codeGenerator/sql/PostgresGenerator.cpp
        src/codeGenerator/sql/SqlGenerator.cpp
        src/codeGenerator/Generator.cpp
        src/helper/Helpers.cpp
        src/variable/Accessor.cpp
        src/variable/Interval.cpp
        src/variable/Variable.cpp
        src/visitor/MyMLVisitor.cpp
        src/visitor/MyPreVisitor.cpp
        src/Main.cpp
        include/mllib/Ml_Lib.h
        )

add_executable(ml2sqlserver
        include/codeGenerator/python/tensorFlow/Graph.h
        include/codeGenerator/python/tensorFlow/MytfVisitor.h
        include/codeGenerator/python/tensorFlow/TFGraphBuilder.h
        include/codeGenerator/python/PythonGenerator.h
        include/codeGenerator/sql/HyperCode.h
        include/codeGenerator/sql/HyperGenerator.h
        include/codeGenerator/sql/PostgresCode.h
        include/codeGenerator/sql/PostgresGenerator.h
        include/codeGenerator/sql/SqlGenerator.h
        include/codeGenerator/sql/SqlOverrideFunctions.h
        include/codeGenerator/Generator.h
        include/helper/Helpers.h
        include/variable/Accessor.h
        include/variable/Interval.h
        include/variable/Variable.h
        include/visitor/MyMLVisitor.h
        include/visitor/MyPreVisitor.h
        antlr4/ML/MLBaseVisitor.cpp
        antlr4/ML/MLBaseVisitor.h
        antlr4/ML/MLLexer.cpp
        antlr4/ML/MLLexer.h
        antlr4/ML/MLParser.cpp
        antlr4/ML/MLParser.h
        antlr4/ML/MLVisitor.cpp
        antlr4/ML/MLVisitor.h
        antlr4/Pre/PreBaseVisitor.cpp
        antlr4/Pre/PreBaseVisitor.h
        antlr4/Pre/PreLexer.cpp
        antlr4/Pre/PreLexer.h
        antlr4/Pre/PreParser.cpp
        antlr4/Pre/PreParser.h
        antlr4/Pre/PreVisitor.cpp
        antlr4/Pre/PreVisitor.h
        antlr4/tf/tfBaseVisitor.cpp
        antlr4/tf/tfBaseVisitor.h
        antlr4/tf/tfLexer.cpp
        antlr4/tf/tfLexer.h
        antlr4/tf/tfParser.cpp
        antlr4/tf/tfParser.h
        antlr4/tf/tfVisitor.cpp
        antlr4/tf/tfVisitor.h
        src/codeGenerator/python/tensorFlow/Graph.cpp
        src/codeGenerator/python/tensorFlow/MytfVisitor.cpp
        src/codeGenerator/python/tensorFlow/TFGraphBuilder.cpp
        src/codeGenerator/python/PythonGenerator.cpp
        src/codeGenerator/sql/HyperGenerator.cpp
        src/codeGenerator/sql/PostgresGenerator.cpp
        src/codeGenerator/sql/SqlGenerator.cpp
        src/codeGenerator/Generator.cpp
        src/helper/Helpers.cpp
        src/variable/Accessor.cpp
        src/variable/Interval.cpp
        src/variable/Variable.cpp
        src/visitor/MyMLVisitor.cpp
        src/visitor/MyPreVisitor.cpp
        src/ServerMain.cpp
        include/mllib/Ml_Lib.h
        )

target_link_libraries(ml2sql antlr4-runtime pthread)
target_link_libraries(ml2sqlserver antlr4-runtime pthread pistache)
