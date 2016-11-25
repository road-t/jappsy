//
//  ViewController.swift
//  OmLauncher
//
//  Created by RT on 16.11.16.
//  Copyright © 2016 RT. All rights reserved.
//

import UIKit
import libGameOM
import libJappsyEngine

class ViewController: UIViewController
{
    var lv: [NSLayoutConstraint] = []
    var lh: [NSLayoutConstraint] = []
    
    var layoutView01: UIView!
    var layoutView02: UIView!
    var gameView: UIView!
    var hiddenView: UIButton!
    
    override var preferredStatusBarStyle: UIStatusBarStyle { return .lightContent }
    
    override func viewDidLoad()
    {
        self.view.backgroundColor! = UIColor.black
        // Menu
        let menuView = UIView()
        menuView.isOpaque = false
        menuView.backgroundColor = UIColor.clear
        menuView.translatesAutoresizingMaskIntoConstraints = false
        self.view.addSubview(menuView)
        
        lv.append(NSLayoutConstraint(item: menuView, attribute: .left, relatedBy: .equal, toItem: self.view, attribute: .left, multiplier: 1.0, constant: 0.0).priority(999))
        
        lh.append(NSLayoutConstraint(item: menuView, attribute: .right, relatedBy: .equal, toItem: self.view, attribute: .left, multiplier: 1.0, constant: 0.0).priority(999))
        self.view.addConstraints(lv)
        self.view.addConstraint(NSLayoutConstraint(item: menuView, attribute: .top, relatedBy: .equal, toItem: self.topLayoutGuide, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(999))
        self.view.addConstraint(NSLayoutConstraint(item: menuView, attribute: .bottom, relatedBy: .equal, toItem: self.view, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(999))
        self.view.addConstraint(NSLayoutConstraint(item: menuView, attribute: .width, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 200.0).priority(999))
        // Menu Item 01
         let menuItem01 = UIButton(type: .custom)
         menuItem01.addTarget(self, action: #selector(self.menuSelect01), for: .touchUpInside)
         menuItem01.isOpaque = false
         menuItem01.backgroundColor = UIColor.blue
         menuItem01.translatesAutoresizingMaskIntoConstraints = false
         menuItem01.setTitle("Menu item 1", for: .normal)
         menuView.addSubview(menuItem01)
         self.view.addConstraint(NSLayoutConstraint(item: menuItem01, attribute: .top, relatedBy: .equal, toItem: menuView, attribute: .top, multiplier: 1.0, constant: 8.0).priority(999))
         self.view.addConstraint(NSLayoutConstraint(item: menuItem01, attribute: .left, relatedBy: .equal, toItem: menuView, attribute: .left, multiplier: 1.0, constant: 8.0).priority(999))
         self.view.addConstraint(NSLayoutConstraint(item: menuItem01, attribute: .right, relatedBy: .equal, toItem: menuView, attribute: .right, multiplier: 1.0, constant: -8.0).priority(999))
         self.view.addConstraint(NSLayoutConstraint(item: menuItem01, attribute: .height, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 50.0).priority(999))
         // Menu Item 02
         let menuItem02 = UIButton(type: .custom)
         menuItem02.addTarget(self, action: #selector(self.menuSelect02), for: .touchUpInside)
         menuItem02.isOpaque = false
         menuItem02.backgroundColor = UIColor.gray
         menuItem02.translatesAutoresizingMaskIntoConstraints = false
         menuItem02.setTitle("Menu item 2", for: .normal)
         menuView.addSubview(menuItem02)
         self.view.addConstraint(NSLayoutConstraint(item: menuItem02, attribute: .top, relatedBy: .equal, toItem: menuItem01, attribute: .bottom, multiplier: 1.0, constant: 8.0).priority(998))
         self.view.addConstraint(NSLayoutConstraint(item: menuItem02, attribute: .left, relatedBy: .equal, toItem: menuView, attribute: .left, multiplier: 1.0, constant: 8.0).priority(999))
         self.view.addConstraint(NSLayoutConstraint(item: menuItem02, attribute: .right, relatedBy: .equal, toItem: menuView, attribute: .right, multiplier: 1.0, constant: -8.0).priority(999))
         self.view.addConstraint(NSLayoutConstraint(item: menuItem02, attribute: .height, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 50.0).priority(999))
        // Menu Item 03
        let menuItem03 = UIButton(type: .custom)
        menuItem03.addTarget(self, action: #selector(self.menuSelect03), for: .touchUpInside)
        menuItem03.isOpaque = false
        menuItem03.backgroundColor = UIColor.blue
        menuItem03.translatesAutoresizingMaskIntoConstraints = false
        menuItem03.setTitle("Барабаны", for: .normal)
        menuView.addSubview(menuItem03)
        self.view.addConstraint(NSLayoutConstraint(item: menuItem03, attribute: .top, relatedBy: .equal, toItem: menuItem02, attribute: .bottom, multiplier: 1.0, constant: 8.0).priority(997))
        self.view.addConstraint(NSLayoutConstraint(item: menuItem03, attribute: .left, relatedBy: .equal, toItem: menuView, attribute: .left, multiplier: 1.0, constant: 8.0).priority(999))
        self.view.addConstraint(NSLayoutConstraint(item: menuItem03, attribute: .right, relatedBy: .equal, toItem: menuView, attribute: .right, multiplier: 1.0, constant: -8.0).priority(999))
        self.view.addConstraint(NSLayoutConstraint(item: menuItem03, attribute: .height, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 50.0).priority(999))
        
         // Layout 01
         layoutView01 = UIView()
         layoutView01.isOpaque = false
         layoutView01.backgroundColor = UIColor.red
         layoutView01.translatesAutoresizingMaskIntoConstraints = false
         self.view.addSubview(layoutView01)
         self.view.addConstraint(NSLayoutConstraint(item: layoutView01, attribute: .top, relatedBy: .equal, toItem: menuView, attribute: .top, multiplier: 1.0, constant: 0.0).priority(998))
         self.view.addConstraint(NSLayoutConstraint(item: layoutView01, attribute: .left, relatedBy: .equal, toItem: menuView, attribute: .right, multiplier: 1.0, constant: 0.0).priority(998))
         self.view.addConstraint(NSLayoutConstraint(item: layoutView01, attribute: .bottom, relatedBy: .equal, toItem: menuView, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(998))
         self.view.addConstraint(NSLayoutConstraint(item: layoutView01, attribute: .width, relatedBy: .equal, toItem: self.view, attribute: .width, multiplier: 1.0, constant: 0.0).priority(998))
         
         let menuButton0 = UIButton(type: .custom)
         menuButton0.addTarget(self, action: #selector(self.showMenu), for: .touchUpInside)
         menuButton0.isOpaque = false
         menuButton0.backgroundColor = UIColor.black
         menuButton0.translatesAutoresizingMaskIntoConstraints = false
         menuButton0.setTitle("Menu", for: .normal)
         layoutView01.addSubview(menuButton0)
         layoutView01.addConstraint(NSLayoutConstraint(item: menuButton0, attribute: .top, relatedBy: .equal, toItem: layoutView01, attribute: .top, multiplier: 1.0, constant: 8.0).priority(999))
         layoutView01.addConstraint(NSLayoutConstraint(item: menuButton0, attribute: .left, relatedBy: .equal, toItem: layoutView01, attribute: .left, multiplier: 1.0, constant: 8.0).priority(999))
         layoutView01.addConstraint(NSLayoutConstraint(item: menuButton0, attribute: .width, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 50.0).priority(999))
         layoutView01.addConstraint(NSLayoutConstraint(item: menuButton0, attribute: .height, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 50.0).priority(999))
         
         // Layout 02
         
         layoutView02 = UIView()
         layoutView02.isOpaque = false
         layoutView02.backgroundColor = UIColor.cyan
         layoutView02.translatesAutoresizingMaskIntoConstraints = false
         self.view.addSubview(layoutView02)
         self.view.addConstraint(NSLayoutConstraint(item: layoutView02, attribute: .top, relatedBy: .equal, toItem: menuView, attribute: .top, multiplier: 1.0, constant: 0.0).priority(998))
         self.view.addConstraint(NSLayoutConstraint(item: layoutView02, attribute: .left, relatedBy: .equal, toItem: menuView, attribute: .right, multiplier: 1.0, constant: 0.0).priority(998))
         self.view.addConstraint(NSLayoutConstraint(item: layoutView02, attribute: .bottom, relatedBy: .equal, toItem: menuView, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(998))
         self.view.addConstraint(NSLayoutConstraint(item: layoutView02, attribute: .width, relatedBy: .equal, toItem: self.view, attribute: .width, multiplier: 1.0, constant: 0.0).priority(998))
         
         let menuButton1 = UIButton(type: .custom)
         menuButton1.addTarget(self, action: #selector(self.showMenu), for: .touchUpInside)
         menuButton1.isOpaque = false
         menuButton1.backgroundColor = UIColor.black
         menuButton1.translatesAutoresizingMaskIntoConstraints = false
         menuButton1.setTitle("Menu", for: .normal)
         layoutView02.addSubview(menuButton1)
         layoutView02.addConstraint(NSLayoutConstraint(item: menuButton1, attribute: .top, relatedBy: .equal, toItem: layoutView02, attribute: .top, multiplier: 1.0, constant: 8.0).priority(999))
         layoutView02.addConstraint(NSLayoutConstraint(item: menuButton1, attribute: .left, relatedBy: .equal, toItem: layoutView02, attribute: .left, multiplier: 1.0, constant: 8.0).priority(999))
         layoutView02.addConstraint(NSLayoutConstraint(item: menuButton1, attribute: .width, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 50.0).priority(999))
         layoutView02.addConstraint(NSLayoutConstraint(item: menuButton1, attribute: .height, relatedBy: .equal, toItem: nil, attribute: .notAnAttribute, multiplier: 1.0, constant: 50.0).priority(999))
 
        
        // gameView
        gameView = UIView()
        gameView.isOpaque = false
        gameView.backgroundColor = UIColor.green
        gameView.translatesAutoresizingMaskIntoConstraints = false
        self.view.addSubview(gameView)
        
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .top, relatedBy: .equal, toItem: menuView, attribute: .top, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .left, relatedBy: .equal, toItem: menuView, attribute: .right, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .bottom, relatedBy: .equal, toItem: menuView, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .width, relatedBy: .equal, toItem: self.view, attribute: .width, multiplier: 1.0, constant: 0.0).priority(998))
        
        // Layout Hidden
        hiddenView = UIButton(type: .custom)
        hiddenView.addTarget(self, action: #selector(self.showLayout), for: .touchUpInside)
        hiddenView.isOpaque = false
        hiddenView.backgroundColor = UIColor.black
        hiddenView.alpha = 0.5
        hiddenView.translatesAutoresizingMaskIntoConstraints = false
        self.view.addSubview(hiddenView)
        self.view.addConstraint(NSLayoutConstraint(item: hiddenView, attribute: .top, relatedBy: .equal, toItem: menuView, attribute: .top, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: hiddenView, attribute: .left, relatedBy: .equal, toItem: menuView, attribute: .right, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: hiddenView, attribute: .bottom, relatedBy: .equal, toItem: menuView, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: hiddenView, attribute: .width, relatedBy: .equal, toItem: self.view, attribute: .width, multiplier: 1.0, constant: 0.0).priority(998))
        layoutView01.isHidden = false
        // layoutView02.isHidden = true
        gameView.isHidden = true
        // Disable Animation Rotation and Keyboard
        //[UIView setAnimationsEnabled:NO];
        // Keep Screen On
        UIApplication.shared.isIdleTimerDisabled = true
        // Black Background With White Status Bar
        self.setNeedsStatusBarAppearanceUpdate()
    }
    
    func showLayout(_ sender: UIButton) {
        self.view.layoutIfNeeded()
        self.view.removeConstraints(lv)
        self.view.addConstraints(lh)
        UIView.animate(withDuration: 0.3, animations: {() -> Void in
            self.hiddenView.alpha = 0.0
            self.view.layoutIfNeeded()
        }, completion: {(_ finished: Bool) -> Void in
            self.hiddenView.isHidden = true
        })
        if !gameView.isHidden {
            omView.updateState(OMVIEW_SHOW)
        }
    }
    
    func showMenu(_ sender: UIButton?)
    {
        self.view.layoutIfNeeded()
        self.view.removeConstraints(lh)
        self.view.addConstraints(lv)
        hiddenView.isHidden = false
        UIView.animate(withDuration: 0.3, animations: {() -> Void in
            self.hiddenView.alpha = 0.5
            self.view.layoutIfNeeded()
        })
    }
    
    func menuSelect01(_ sender: UIButton)
    {
        layoutView01.isHidden = false
        
        gameView.isHidden = true
        self.showLayout(sender)
    }
    
    func menuSelect02(_ sender: UIButton)
    {
        layoutView01.isHidden = true
        
        gameView.isHidden = true
        self.showLayout(sender)
    }
    
    func onCloseCallback(userData: Void)
    {
        let controller = (userData as! ViewController)
        controller.showMenu(nil)
    }
    
    func menuSelect03(_ sender: UIButton)
    {
        if omView == nil
        {
            /*var basePath = "https://dev03-om.jappsy.com/jappsy/"
             // Demo server
             //NSString* basePath = @"https://om.jappsy.com/jappsy/"; // Production server
             var token = "e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3"
             var sessid = "8ea5f70b15263872760d7e14ce8e579a"
             var devid = ""
             var locale = "RU"
             omView = OMView.init(basePath, token: token, sessid: sessid, devid: devid, locale: locale, onclose: onCloseCallback, userData: (self as! Void))*/
            
            let basePath = IS_DEV ? "https://dev03-om.jappsy.com/jappsy/" : "https://om.jappsy.com/jappsy/"
            let token = "e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3"
            let sessid = "8ea5f70b15263872760d7e14ce8e579a"
            let devid = ""
            let locale = "RU"
            
            // CGRect(x: 0, y: 300, width: 400, height: 300)
            
            omView = OMView.init(basePath, token: token, sessid: sessid, devid: devid, locale: locale, onclose: nil, userData: nil)
            
            gameView.addSubview(omView)
            gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .top, relatedBy: .equal, toItem: gameView, attribute: .top, multiplier: 1.0, constant: 0.0).priority(999))
            gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .left, relatedBy: .equal, toItem: gameView, attribute: .left, multiplier: 1.0, constant: 0.0).priority(999))
            gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .bottom, relatedBy: .equal, toItem: gameView, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(999))
            gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .right, relatedBy: .equal, toItem: gameView, attribute: .right, multiplier: 1.0, constant: 0.0).priority(999))
            omView.onStart()
            omView.updateState(OMVIEW_RUN)
        }
        layoutView01.isHidden = true
        layoutView02.isHidden = true
        gameView.isHidden = false
        self.showLayout(sender)
    }
    
    override func awakeFromNib()
    {
        super.awakeFromNib()
        UIDevice.current.beginGeneratingDeviceOrientationNotifications()
        //      NotificationCenter.default.addObserver(self, selector: #selector(self.orientationChanged), name: .didChangeNotification, object: nil)
    }
    
    func orientationChanged(_ notification: Notification)
    {
        omView.orientationChanged(notification)
    }
}

