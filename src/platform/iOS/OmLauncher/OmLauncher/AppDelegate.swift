//
//  AppDelegate.swift
//  OmLauncher
//
//  Created by RT on 16.11.16.
//  Copyright © 2016 RT. All rights reserved.
//

import UIKit
import libJappsyEngine
import libGameOM

let IS_DEV = true

var omView: OMView!

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate
{

    var window: UIWindow?

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplicationLaunchOptionsKey: Any]?) -> Bool
    {
        self.window?.makeKeyAndVisible()
        
        var paths = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)
        var cacheDirectory = paths[0]
        jappsyInit(String(cacheDirectory.utf8), nil)

        return true
    }

    func applicationWillResignActive(_ application: UIApplication)
    {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
        if omView != nil
        {
            omView!.updateState(OMVIEW_STOP)
        }

    }

    func applicationDidEnterBackground(_ application: UIApplication)
    {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
        if omView != nil
        {
            omView!.updateState(OMVIEW_STOP)
        }
    }

    func applicationWillEnterForeground(_ application: UIApplication)
    {
        // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
        if omView != nil
        {
            omView!.updateState(OMVIEW_RUN)
        }

    }

    func applicationDidBecomeActive(_ application: UIApplication)
    {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
        if omView != nil
        {
            omView!.updateState(OMVIEW_RUN)
        }
    }

    func applicationWillTerminate(_ application: UIApplication)
    {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
        if omView != nil
        {
            omView!.onStop()
        }
    }

    func applicationDidReceiveMemoryWarning(_ application: UIApplication)
    {
        if omView != nil
        {
            omView!.onStop()
        }
    }
}

