#
# system import
import time
import string
import errno

import rexec
_rexec = rexec.RExec()

def eval(text):
    return _rexec.r_eval(text)

# enstore imports
import enstore_functions2
import enstore_constants
import e_errors
import Trace

# key to get at a server supplied short text string
SHORT_TEXT = "short_text"

DEFAULT_PID = -1
DEFAULT_UID = ""
DEFAULT_SOURCE = "None"

SEVERITY = "severity"

MATCH = 1
NO_MATCH = 0

PATROL_SEVERITY = { e_errors.sevdict[e_errors.ALARM] : '4',
                    e_errors.sevdict[e_errors.ERROR] : '4',
                    e_errors.sevdict[e_errors.USER_ERROR] : '3',
                    e_errors.sevdict[e_errors.WARNING] : '2',
                    e_errors.sevdict[e_errors.INFO] : '1',
                    e_errors.sevdict[e_errors.MISC] : '1'
                    }

class GenericAlarm:

    def __init__(self):
        self.timedate = time.time()
        self.id = str(self.timedate)
        self.host = ""
        self.pid = DEFAULT_PID
        self.uid = DEFAULT_UID
        self.source = DEFAULT_SOURCE
        self.severity = e_errors.DEFAULT_SEVERITY
        self.root_error = e_errors.DEFAULT_ROOT_ERROR
        self.alarm_info = {}
        self.patrol = 0
	self.num_times_raised = 1L

    def split_severity(self, sev):
	l = string.split(sev)
	sev = l[0]
	if len(l) == 2:
	    tmp = string.replace(l[1], '(', '')
	    tmp = string.replace(tmp, ')', '')
	    num_times_raised = string.atol(tmp)
	else:
	    num_times_raised = 1
	return sev, num_times_raised

    # output the alarm for patrol
    def prepr(self):
        self.patrol = 1
        alarm = repr(self)
        self.patrol = 0
        return alarm

    def seen_again(self):
	try:
	    self.num_times_raised = self.num_times_raised + 1
	except errno.errorcode[errno.EOVERFLOW]:
	    self.num_times_raised = -1

    # return the a list of the alarm pieces we need to output
    def list_alarm(self):
	return [self.id, self.host, self.pid, self.uid, 
		"%s (%s)"%(self.severity, self.num_times_raised),
		self.source, self.root_error, self.alarm_info]

    # output the alarm
    def __repr__(self):
        # format ourselves to be a straight ascii line of the same format as
        # mentioned above
        if self.patrol:
            host = string.split(self.host,".")
            # enstore's severities must be mapped to patrols'
            sev = PATROL_SEVERITY[self.severity]
            return string.join((host[0], "Enstore" , sev, self.short_text(),
                                "\n"))
        else:
            return repr(self.list_alarm())

    # format the alarm as a simple, short text string to use to signal
    # that there is something wrong that needs further looking in to
    def short_text(self):
        # ths simple string has the following format -
        #         servername on node - text string
        # where servername and node are replaced with the appropriate values
        str = "%s on %s at %s - "%(self.source, self.host,
                                  enstore_functions2.format_time(self.timedate))

        # look in the info dict.  if there is a key "short_text", use it to get
        # the text, else use default text just signaling a problem
        return str+self.alarm_info.get(SHORT_TEXT, "%s %s"%(self.root_error,
                                                            self.alarm_info))

    # compare the passed in info to see if it the same as that of the alarm
    def compare(self, host, severity, root_error, source, alarm_info):
        if (self.host == host and 
            self.root_error == root_error and
            self.severity == severity and
            self.source == source):
	    # now that all that is done we can compare the dictionary to see
	    # if it is the same
	    if len(alarm_info) == len(self.alarm_info):
		keys = self.alarm_info.keys()
		for key in keys:
		    if alarm_info.has_key(key):
			if not self.alarm_info[key] == alarm_info[key]:
			    # we found something that does not match
			    break
		    else:
			# there is no corresponding key
			break
		else:
		    # all keys matched between the two dicts
		    return MATCH

		return NO_MATCH
	    return NO_MATCH
        return NO_MATCH

    # return the alarms unique id
    def get_id(self):
        return self.id

class Alarm(GenericAlarm):

    def __init__(self, host, severity, root_error, pid, uid, source,
                 alarm_info=None):
        GenericAlarm.__init__(self)

        if alarm_info is None:
            alarm_info = {}
        self.host = host
        self.severity = severity
        self.root_error = root_error
        self.pid = pid
        self.uid = uid
        self.source = source
        self.alarm_info = alarm_info

class AsciiAlarm(GenericAlarm):

    def __init__(self, text):
        GenericAlarm.__init__(self)

	# sometimes the alarm file has junk in it. so protect against that
	try:
	    [self.id, self.host, self.pid, self.uid, sev,
	     self.source, self.root_error, self.alarm_info] = eval(text)
	    self.severity, self.num_times_raised = self.split_severity(sev)
	except TypeError:
	    self.id = 0    # not a valid alarm

class LogFileAlarm(GenericAlarm):

    def unpack_dict(self, dict):
	if dict.has_key(enstore_constants.ROOT_ERROR):
	    self.root_error = dict[enstore_constants.ROOT_ERROR]
	    del dict[enstore_constants.ROOT_ERROR]
	else:
	    self.root_error = "UNKNOWN"
	if dict.has_key(SEVERITY):
	    self.severity = dict[SEVERITY]
	    del dict[SEVERITY]
	else:
	    self.severity = e_errors.ALARM
	self.alarm_info = dict

    def __init__(self, text, date):
	GenericAlarm.__init__(self)

	self.num_times_raised = 1

	# get rid of the MSG_TYPE part of the alarm
	[t, self.host, self.pid, self.uid, dummy, self.source,
	 text] = string.split(text, " ", 6)

        # we need to get rid of the MSG_TYPE text.  it may be at the
        # beginning or the end.
        text = string.replace(text, Trace.MSG_ALARM, "")

	# assemble the real timedate
	self.timedate = time.strptime("%s %s"%(date, t), "%Y-%m-%d %H:%M:%S")
        # i am doing this explicitly because it seems that time.strptime will
        # return 0 for the DST flag even if it is DST
        self.timedate = (self.timedate[0], self.timedate[1],
                         self.timedate[2], self.timedate[3],
                         self.timedate[4], self.timedate[5],
                         self.timedate[6], self.timedate[7],
                         time.daylight)
	self.id = str(self.timedate)
	
	text = string.strip(text)
	# text may be only a dictionary or it may be of the following format -
	#       root-error, {...} (severity : n)
	if text[0] == "{":
	    dict = eval(text)
	    # split up the dictionary into components
	    self.unpack_dict(dict)
	else:
	    index = string.find(text, ", {")
	    if index == -1:
		# we could not find the substring, punt
		self.root_error = text
		self.severity = e_errors.ALARM
		self.alarm_info = {}
	    else:
		self.root_error = text[0:index]
		# now pull out any dictionary, skip the ", "
		index = index + 2
		end_index = string.find(text, "} (")
		if end_index == -1:
		    # couldn't find it, punt again
		    self.severity = e_errors.ALARM
		    self.alarm_info = text[index:]
		else:
		    dict = eval(text[index:end_index+1])
		    self.alarm_info = dict
		    # now get the severity
		    index = string.rfind(text, ")")
		    if index == -1:
			# could not find it
			self.severity = e_errors.ALARM
		    else:
			sev = text[index -1]
			for k,v in e_errors.sevdict.items():
			    if v == sev:
				self.severity = k
				break
			else:
			    # there was no match
			    self.severity = e_errors.ALARM
