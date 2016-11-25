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
    var gameView: UIView!
    var reloadButton: UIButton!
    
    override var preferredStatusBarStyle: UIStatusBarStyle { return .lightContent }
    
    override func viewDidLoad()
    {
        self.view.backgroundColor = UIColor.black
        
        // gameView
        gameView = UIView()
        gameView.isOpaque = false
        gameView.backgroundColor = UIColor.green
        gameView.translatesAutoresizingMaskIntoConstraints = false
        self.view.addSubview(gameView)
        
        // omView won't work w/o it's container constraints
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .top, relatedBy: .equal, toItem: self.view, attribute: .top, multiplier: 1.0, constant: UIApplication.shared.statusBarFrame.height).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .left, relatedBy: .equal, toItem: self.view, attribute: .left, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .bottom, relatedBy: .equal, toItem: self.view, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(998))
        self.view.addConstraint(NSLayoutConstraint(item: gameView, attribute: .width, relatedBy: .equal, toItem: self.view, attribute: .width, multiplier: 1.0, constant: 0.0).priority(998))
        
        // don't fade screen to black
        UIApplication.shared.isIdleTimerDisabled = true
        
        self.startGame()
    }
    
    func startGame()
    {
       omView = OMView.init(
            IS_DEV ? "https://dev03-om.jappsy.com/jappsy/" : "https://om.jappsy.com/jappsy/",
            token: "e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3",
            sessid: "8ea5f70b15263872760d7e14ce8e579a",
            devid: "",
            locale: "RU",
            onclose: nil,
            userData: nil
        )
        
        gameView.addSubview(omView)
        
        gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .top, relatedBy: .equal, toItem: gameView, attribute: .top, multiplier: 1.0, constant: 0.0).priority(999))
        gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .left, relatedBy: .equal, toItem: gameView, attribute: .left, multiplier: 1.0, constant: 0.0).priority(999))
        gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .bottom, relatedBy: .equal, toItem: gameView, attribute: .bottom, multiplier: 1.0, constant: 0.0).priority(999))
        gameView.addConstraint(NSLayoutConstraint(item: omView, attribute: .right, relatedBy: .equal, toItem: gameView, attribute: .right, multiplier: 1.0, constant: 0.0).priority(999))
        
        omView.onStart()
        omView.updateState(OMVIEW_RUN)
        
        omView.updateState(OMVIEW_SHOW)
    }
}

