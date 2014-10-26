/* Including the Node and V8 dependencies */
#include <node.h>
#include <v8.h>

/* Including the FSEvent Module */
#include "fsevents.cpp"

using namespace v8;

/* 
    Function template for our listener function. Arguments
        -Path: String
        -Callback: function(eventObj)
 */
Handle<Value> Listener(const Arguments& args) {
	HandleScope scope; 

    /* Checks if the arguments' types are correct*/
	if (!args[0]->IsString()) {
		return ThrowException(Exception::TypeError(
			String::New("First argument must be a string.")));
	}

	if (!args[1]->IsFunction()) {
		return ThrowException(Exception::TypeError(
			String::New("Second argument must be a function")));
	}

    /* Type-casting from v8::String to char * as we can use it in FSEvent module */
	String::AsciiValue string(args[0]);
	char * path = (char *) malloc(string.length() + 1);
	strcpy(path, *string);

    /* Starting the event stream */
	startListening(path, Local<Function>::Cast(args[1]));

	return scope.Close(Undefined());
}

/* Setting up the module with the listen function */
void init(Handle<Object> target) {
	NODE_SET_METHOD(target, "listen", Listener);
}

NODE_MODULE(FSEvent, init);