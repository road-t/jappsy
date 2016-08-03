//
//  Shader.fsh
//  JappsyExample
//
//  Created by VipDev on 03.08.16.
//  Copyright © 2016 jappsy.com. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
