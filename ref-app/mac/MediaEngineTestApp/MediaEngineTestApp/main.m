//
//  main.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-5.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
#ifdef ENABLE_COMMAND_LINE
    if (argc < 2) {
        printf("usage: app_name [-c HOST_IP][-h][-n][-s SEVER_IP] \n");
        printf("       -c: set the app role to CLIENT  \n");
        printf("       HOST_IP: host IP. for example:10.224.35.26   \n");
        printf("       -h: set the app role to HOST \n");
        printf("       -n: enable the no render mode \n");
        printf("       -s: enable the system log \n");
        printf("       SEVER_IP: system log server IP. for example:10.224.53.63 \n");
        printf("For example: app_name -c 10.224.52.35 -n -s 15.265.35.26 \n");
    }
#endif
    return NSApplicationMain(argc, (const char **)argv);
}
