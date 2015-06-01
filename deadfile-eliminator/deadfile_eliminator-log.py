# Copyright (C) 2015 TOSHIBA corp.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
import pyinotify
import sys

wm = pyinotify.WatchManager()
mask = pyinotify.IN_ACCESS

class EventHandler(pyinotify.ProcessEvent):
    def process_IN_ACCESS(self, event):
        print event.pathname

handler = EventHandler()
notifier = pyinotify.Notifier(wm, handler)
wdd = wm.add_watch(sys.argv[1], mask, rec=True)

notifier.loop()
