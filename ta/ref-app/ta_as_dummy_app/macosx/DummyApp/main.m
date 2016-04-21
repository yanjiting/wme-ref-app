//
//  main.m
//  DummyApp
//
//  Created by vagouzhou on 8/11/14.
//  Copyright (c) 2014 wx2. All rights reserved.
//

#import <Cocoa/Cocoa.h>


#ifdef __cplusplus
extern "C"
{
#endif
    
	void InitAgentByInputParams(int argc, const char ** argv);
	void CleanupAgent();
    
#ifdef __cplusplus
}
#endif

int main(int argc, const char * argv[])
{
    
	InitAgentByInputParams( argc, argv);
    int ret = NSApplicationMain(argc, argv);
	CleanupAgent();

    return ret;
}
