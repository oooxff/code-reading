// DOVEBrowserApplet.java,v 1.4 1999/02/07 20:37:01 cdgill Exp
// 
// = FILENAME
//   DOVEBrowserApplet.java
//
// = AUTHOR
//   Michael Kircher (mk1@cs.wustl.edu)
//
// = DESCRIPTION
//   Entry point for the demo when started inside an applet.
//
// ============================================================================


import java.awt.*;

public class DOVEBrowserApplet extends java.applet.Applet {

  DemoCore demoCore_;
  static String nameServiceIOR_ = null;
  
  public DOVEBrowserApplet () {
    super (); 
  }
  
  public void init () {
    // in case of an applet use the following line, else not!
    nameServiceIOR_ = this.getParameter ("NSIOR");
    
    if (nameServiceIOR_ == null) {
      System.out.println ("The DOVE Browser cannot run, because no Naming Service IOR was found!");
      return;
    }

    demoCore_ = new DemoCore (nameServiceIOR_,  // name service IOR
			      null,             // name service port
			      null,             // args
                              false,            // don't use queueing
			      this);            // pointer to the applet
    demoCore_.show ();
    demoCore_.run ();
  }
}
  
  






