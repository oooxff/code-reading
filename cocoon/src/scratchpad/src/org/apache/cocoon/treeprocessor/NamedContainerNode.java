/*****************************************************************************
 * Copyright (C) The Apache Software Foundation. All rights reserved.        *
 * ------------------------------------------------------------------------- *
 * This software is published under the terms of the Apache Software License *
 * version 1.1, a copy of which has been included  with this distribution in *
 * the LICENSE file.                                                         *
 *****************************************************************************/

package org.apache.cocoon.treeprocessor;

import org.apache.cocoon.components.pipeline.EventPipeline;
import org.apache.cocoon.components.pipeline.StreamPipeline;
import org.apache.cocoon.environment.Environment;

/**
 * A named container node that just invokes its children.
 *
 * @author <a href="mailto:sylvain@apache.org">Sylvain Wallez</a>
 * @version CVS $Revision: 1.2 $ $Date: 2002/01/15 11:10:52 $
 */

public class NamedContainerNode extends ContainerNode implements NamedProcessingNode {

    private String name;

    public NamedContainerNode(String name) {
        this.name = name;
    }
    
    public String getName() {
        return this.name;
    }
}
