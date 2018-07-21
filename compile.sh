#!/bin/sh

reset

mkdir bin

cd bin

if [ "$1" == "debug" ]; then
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/exec_state.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/DefaultObjectDefineUtil.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/FileUrl.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/Context.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/TokenStream.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/Parser.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/string.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/TreeObjectMap.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/ASTExecuter.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/DebugUtils.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/TokenNamespace.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Integer.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Scope.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Null.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Undefined.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/NativeFunction.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/ObjectConverter.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Boolean.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/CodeFunction.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/StringType.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Double.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Object.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Array.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/VirtualObject.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/GarbageCollector.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/NativeLoaderType.cpp
	g++ -rdynamic -w -g -std=c++11 -fpermissive -c ../src/objects/Error.cpp
	
	cd ../

	g++ -rdynamic -w -g -std=c++11 -fpermissive src/ck.cpp bin/exec_state.o bin/DefaultObjectDefineUtil.o bin/FileUrl.o bin/Context.o bin/TokenStream.o bin/Parser.o bin/string.o bin/TreeObjectMap.o bin/ASTExecuter.o bin/DebugUtils.o bin/TokenNamespace.o bin/Integer.o bin/Scope.o bin/Null.o bin/Undefined.o bin/NativeFunction.o bin/ObjectConverter.o bin/Boolean.o bin/CodeFunction.o bin/StringType.o bin/Double.o bin/Object.o bin/Array.o bin/VirtualObject.o bin/GarbageCollector.o bin/NativeLoaderType.o bin/Error.o -ldl -o bin/ck
	
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./bin/ck -f res/in.ck 2> erroutput.txt
elif [ "$1" == "install" ]; then
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/exec_state.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/DefaultObjectDefineUtil.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/FileUrl.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/Context.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/TokenStream.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/Parser.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/string.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/TreeObjectMap.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/ASTExecuter.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/DebugUtils.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/TokenNamespace.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Integer.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Scope.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Null.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Undefined.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/NativeFunction.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/ObjectConverter.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Boolean.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/CodeFunction.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/StringType.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Double.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Object.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Array.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/VirtualObject.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/GarbageCollector.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/NativeLoaderType.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Error.cpp
	
	cd ../

	g++ -rdynamic -O -w -g -std=c++11 -fpermissive src/ck.cpp bin/exec_state.o bin/DefaultObjectDefineUtil.o bin/FileUrl.o bin/Context.o bin/TokenStream.o bin/Parser.o bin/string.o bin/TreeObjectMap.o bin/ASTExecuter.o bin/DebugUtils.o bin/TokenNamespace.o bin/Integer.o bin/Scope.o bin/Null.o bin/Undefined.o bin/NativeFunction.o bin/ObjectConverter.o bin/Boolean.o bin/CodeFunction.o bin/StringType.o bin/Double.o bin/Object.o bin/Array.o bin/VirtualObject.o bin/GarbageCollector.o bin/NativeLoaderType.o bin/Error.o -ldl -o bin/ck
	sudo cp bin/ck /usr/local/bin/ck
elif [ "$1" == "clean" ]; then
	rm *
else
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/exec_state.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/DefaultObjectDefineUtil.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/FileUrl.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/Context.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/TokenStream.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/Parser.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/string.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/TreeObjectMap.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/ASTExecuter.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/DebugUtils.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/TokenNamespace.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Integer.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Scope.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Null.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Undefined.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/NativeFunction.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/ObjectConverter.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Boolean.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/CodeFunction.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/StringType.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Double.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Object.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Array.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/VirtualObject.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/GarbageCollector.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/NativeLoaderType.cpp
	g++ -rdynamic -O -w -g -std=c++11 -fpermissive -c ../src/objects/Error.cpp
	
	cd ../

	g++ -rdynamic -O -w -g -std=c++11 -fpermissive src/ck.cpp bin/exec_state.o bin/DefaultObjectDefineUtil.o bin/FileUrl.o bin/Context.o bin/TokenStream.o bin/Parser.o bin/string.o bin/TreeObjectMap.o bin/ASTExecuter.o bin/DebugUtils.o bin/TokenNamespace.o bin/Integer.o bin/Scope.o bin/Null.o bin/Undefined.o bin/NativeFunction.o bin/ObjectConverter.o bin/Boolean.o bin/CodeFunction.o bin/StringType.o bin/Double.o bin/Object.o bin/Array.o bin/VirtualObject.o bin/GarbageCollector.o bin/NativeLoaderType.o bin/Error.o -ldl -o bin/ck
	
	./bin/ck -f res/in.ck
fi
