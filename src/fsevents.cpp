
/* Compile with con gcc -framework CoreServices -lstdc++ main.cpp -o main */
#include <CoreServices/CoreServices.h>

#include <node.h>
#include <v8.h>

using namespace v8;

/* Global container for the listen function callback */
Local<Function> callback;

int eventModified = kFSEventStreamEventFlagItemFinderInfoMod |
                    kFSEventStreamEventFlagItemModified |
                    kFSEventStreamEventFlagItemInodeMetaMod |
                    kFSEventStreamEventFlagItemChangeOwner |
                    kFSEventStreamEventFlagItemXattrMod;

int eventRenamed = kFSEventStreamEventFlagItemCreated |
                   kFSEventStreamEventFlagItemRemoved |
                   kFSEventStreamEventFlagItemRenamed;

int eventSystem = kFSEventStreamEventFlagUserDropped |
                  kFSEventStreamEventFlagKernelDropped |
                  kFSEventStreamEventFlagEventIdsWrapped |
                  kFSEventStreamEventFlagHistoryDone |
                  kFSEventStreamEventFlagMount |
                  kFSEventStreamEventFlagUnmount |
                  kFSEventStreamEventFlagRootChanged;

/* Executed everytime a change has been made. Also creates a v8::Object which is passed as an argument in the callback function */
void myCallbackFunction(

    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[]

){

    int i;
    char **paths = (char **)eventPaths;

    for( i=0; i < (int)numEvents; i++ ){

      /* Creates the object and sets its properties */
      Local<Object> event = Object::New();
      event->Set(String::NewSymbol("path"), String::New(paths[i]));

      if (eventFlags[i] & eventModified)
        event->Set(String::NewSymbol("modified"), True());
      else
        event->Set(String::NewSymbol("modified"), False());

      if (eventFlags[i] & kFSEventStreamEventFlagItemCreated)
        event->Set(String::NewSymbol("created"), True());
      else
        event->Set(String::NewSymbol("created"), False());

      if (eventFlags[i] & kFSEventStreamEventFlagItemRemoved)
        event->Set(String::NewSymbol("removed"), True());
      else
        event->Set(String::NewSymbol("removed"), False());

      if (eventFlags[i] & kFSEventStreamEventFlagItemRenamed)
        event->Set(String::NewSymbol("renamed"), True());
      else
        event->Set(String::NewSymbol("renamed"), False());

      if (eventFlags[i] & eventSystem)
        event->Set(String::NewSymbol("System"), True());
      else
        event->Set(String::NewSymbol("System"), False());

      const unsigned argc = 1;
      Local<Value> argv[argc] = { Local<Value>::New(event) };
      callback->Call(Context::GetCurrent()->Global(), argc, argv);

    }

}

void startListening(char *path, Local<Function> cb){

    /* Define variables and create a CFArray object containing CFString objects containing paths to watch. */
    CFStringRef mypath              = CFStringCreateWithCString( NULL, path, kCFStringEncodingUTF8);
    CFArrayRef pathsToWatch         = CFArrayCreate( NULL, ( const void ** ) &mypath, 1, NULL );
    CFAbsoluteTime latency          = 3.0; /* Latency in seconds */
    FSEventStreamRef stream;

    callback = Local<Function>::Cast(cb);

    /* Create the stream, passing in a callback */
    stream = FSEventStreamCreate(

        NULL,
        &myCallbackFunction,
        NULL, // could put stream-specific data here. FSEventStreamRef stream;*/
        pathsToWatch,
        kFSEventStreamEventIdSinceNow, /* Or a previous event ID */
        latency,
        kFSEventStreamCreateFlagFileEvents /* Flags explained in reference: https://developer.apple.com/library/mac/documentation/Darwin/Reference/FSEvents_Ref/Reference/reference.html */

    );

    /* Create the stream before calling this. */
    FSEventStreamScheduleWithRunLoop( stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
    FSEventStreamStart( stream );
    CFRunLoopRun();

}
