import pyinotify
import sys
from mailer import send_sfm_report

wm = pyinotify.WatchManager()
mask = pyinotify.IN_ACCESS | pyinotify.IN_ATTRIB
#IN_ACCESS	Yes	file was accessed.
#IN_ATTRIB	Yes	metadata changed.
#IN_CLOSE_NOWRITE	Yes	unwrittable file was closed.
#IN_CLOSE_WRITE	Yes	writtable file was closed.
#IN_CREATE	Yes	file/dir was created in watched directory.
#IN_DELETE	Yes	file/dir was deleted in watched directory.
#IN_DELETE_SELF	Yes	watched item itself was deleted.
#IN_DONT_FOLLOW	No	don't follow a symlink (lk 2.6.15).
#IN_IGNORED	Yes	raised on watched item removing. Probably useless for you, prefer instead IN_DELETE*.
#IN_ISDIR	No	event occurred against directory. It is always piggybacked to an event. The Event structure automatically provide this information (via .is_dir)
#IN_MASK_ADD	No	to update a mask without overwriting the previous value (lk 2.6.14). Useful when updating a watch.
#IN_MODIFY	Yes	file was modified.
#IN_MOVE_SELF	Yes	watched item itself was moved, currently its full pathname destination can only be traced if its source directory and destination directory are both watched. Otherwise, the file is still being watched but you cannot rely anymore on the given path (.path)
#IN_MOVED_FROM	Yes	file/dir in a watched dir was moved from X. Can trace the full move of an item when IN_MOVED_TO is available too, in this case if the moved item is itself watched, its path will be updated (see IN_MOVE_SELF).
#IN_MOVED_TO	Yes	file/dir was moved to Y in a watched dir (see IN_MOVE_FROM).
#IN_ONLYDIR	No	only watch the path if it is a directory (lk 2.6.15). Usable when calling .add_watch.
#IN_OPEN	Yes	file was opened.
#IN_Q_OVERFLOW	Yes	event queued overflowed. This event doesn't belongs to any particular watch.
#IN_UNMOUNT	Yes	backing fs was unmounted. Notified to all watches located on this fs.

class EventHandler(pyinotify.ProcessEvent):
    #def process_IN_ACCESS(self, event):
        #send_sfm_report(event.pathname)
    def process_default(self, event):
        send_sfm_report(event.pathname)

handler = EventHandler()
notifier = pyinotify.Notifier(wm, handler)
wdd = wm.add_watch(sys.argv[1], mask, rec=True)

notifier.loop()
