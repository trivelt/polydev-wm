/* PolyDev WM by Maciej Michalec is based on the TinyWM.
 * TinyWM is written by Nick Welch <mack@incise.org>, 2005.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
FILE *logfile;
char terminalApp[64];

void readConfiguration() 
{
    char key[64];
	char value[64];

 	FILE* config = fopen("/etc/polydevWM.conf","r");
	while (fscanf(config, "%s %s\n", &key[0], &value[0]) != EOF)
	{
		if (strcmp(key, "terminal-app") == 0)
        {
            strcpy(terminalApp, value);
        }
	}
    fclose(config);
}

int main()
{
    readConfiguration();
    logfile = fopen("/tmp/polydevWM.log", "a+");
    Display * dpy;
    Window root;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;
    XEvent msg; // msg to send (alt+f4)
    short focusedOnWindow;

    if (!(dpy = XOpenDisplay(0x0))) return 1;

    root = DefaultRootWindow(dpy);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F1")), Mod1Mask, root,
            True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("t")), ControlMask | Mod1Mask, root,
             True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F4")), Mod1Mask, root,
             True, GrabModeAsync, GrabModeAsync);
    XGrabButton(dpy, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);

    for(;;)
    {
        XNextEvent(dpy, &ev);
        // fprintf(logfile, "Event captured, type=%d\n", ev.type);
        focusedOnWindow = (ev.type == KeyPress && ev.xkey.subwindow != None);
        if (focusedOnWindow && ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("F4")))
        {
            memset(&msg, 0, sizeof(msg));
            msg.xclient.type = ClientMessage;
            msg.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
            msg.xclient.window = ev.xkey.subwindow;
            msg.xclient.format = 32;
            msg.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
            msg.xclient.data.l[1] = CurrentTime;
            XSendEvent(dpy, ev.xkey.subwindow, False, 0, &msg);
        }
        else if (focusedOnWindow && ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("F1")))
        {
            XRaiseWindow(dpy, ev.xkey.subwindow);
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("t")))
        {
            system(terminalApp);
        }
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {
            XGrabPointer(dpy, ev.xbutton.subwindow, True,
            PointerMotionMask|ButtonReleaseMask, GrabModeAsync,
            GrabModeAsync, None, None, CurrentTime);
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        else if (ev.type == MotionNotify)
        {
            int xdiff, ydiff;
            while (XCheckTypedEvent(dpy, MotionNotify, &ev));
            xdiff = ev.xbutton.x_root - start.x_root;
            ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, ev.xmotion.window,
                attr.x + (start.button==1 ? xdiff : 0),
                attr.y + (start.button==1 ? ydiff : 0),
                MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
        }
        else if (ev.type == ButtonRelease)
        {
            XUngrabPointer(dpy, CurrentTime);
        }
    }
    fclose(logfile);
}

