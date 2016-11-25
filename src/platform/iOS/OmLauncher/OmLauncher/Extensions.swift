//
//  Extensions.swift
//  OmLauncher
//
//  Created by RT on 23.11.16.
//  Copyright © 2016 RT. All rights reserved.
//

import UIKit

@available(iOS 8.0, *)

public extension NSLayoutConstraint
{
    func priority(_ priority: Float) -> NSLayoutConstraint
    {
        self.priority = priority
        return self
    }
}
