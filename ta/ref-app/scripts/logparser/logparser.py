#!/usr/bin/env python
import re
import os
import sys
import time
import glob
import shutil
import itertools
import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
from collections import namedtuple, OrderedDict
from dateutil import parser as dt_parser
from copy import deepcopy
from matplotlib import dates
import argparse
import logging
import datetime
from logging import FileHandler, StreamHandler


# utilize logging to output to both file/console
LOG_FILE_PATH = "wme_analyze.txt"
default_formatter = logging.Formatter("%(message)s")
console_handler = StreamHandler()
console_handler.setFormatter(default_formatter)
file_handler = FileHandler(LOG_FILE_PATH, "w")
file_handler.setLevel(logging.INFO)
file_handler.setFormatter(default_formatter)

logger = logging.getLogger()
logger.addHandler(console_handler)
logger.addHandler(file_handler)
logger.setLevel(logging.DEBUG)
# use rst format for analyzing output, todo: disable output to console
# logger.info('.. contents::\n')

args = None

# color_cycle = itertools.cycle(plt.cm.spectral(np.linspace(0, 1, 10)))
color_cycle = itertools.cycle(['#a6cee3', '#1f78b4', '#b2df8a', '#33a02c', '#fb9a99', '#e31a1c',
                               '#fdbf6f', '#ff7f00', '#cab2d6', '#6a3d9a', '#ffff99', '#b15928'])
stress_color_cycle = itertools.cycle(['#fd8d3c', '#7a0177', '#006837', '#253494', '#993404', '#bd0026'])
pause = lambda: raw_input('\nPress Enter to continue')


def convert_to_utc(orig, offset):
    if offset != 0:
        delta = datetime.timedelta(hours=-offset)
        dst = orig + delta
    else:
        dst = orig
    return dst


def log_header0(title):
    logger.info('\n'+'#'*160)
    logger.info(title)
    logger.info('#'*160+'\n')


def log_header1(title):
    logger.info('\n%s' % title)
    logger.info('#'*80+'\n')


def log_header2(title):
    logger.info('\n%s' % title)
    logger.info('='*80+'\n')


def log_header3(title):
    logger.info(title)
    logger.info('-'*80+'\n')


def type_convert(s):
    try:
        return int(s)
    except ValueError:
        try:
            return float(s)
        except ValueError:
            return s


def dt_to_epoch_ms(dt):
    return time.mktime(dt.timetuple()) * 1000 + dt.microsecond / 1000


class PlotCfg(object):

    def __init__(self, subplot_idx=0, ax_idx=0, color=None, linestyle='-', xticks=None, yticks=None, units=''):
        self.subplot_idx = subplot_idx
        self.ax_idx = ax_idx
        if color is None:
            color = color_cycle.next()
        self.color = color
        self.linestyle = linestyle
        self.xticks = xticks
        self.yticks = yticks
        self.units = units


class MetricsEventsFilter(object):

    def __call__(self, ts, val):
        return True


class MetricsEventsFilterByDatetime(MetricsEventsFilter):

    def __init__(self, start_ts, end_ts):
        self.start_ts = start_ts
        self.end_ts = end_ts

    def __call__(self, ts, val):
        if (self.start_ts is not None and ts < self.start_ts) or \
           (self.end_ts is not None and ts > self.end_ts):
            return False
        else:
            return True


class MetricsEventsTransformer(object):

    def __call__(self, metric):
        return metrics


class MetricsEventsTimestampTransformer(MetricsEventsTransformer):

    def __init__(self, offset):
        self.offset = offset

    def __call__(self, metric):
        metric.ts = [convert_to_utc(item, self.offset) for item in metric.ts]
        # print 'transform by offset, ts = ', metric.ts


class RawDataBase(object):

    def __init__(self, label, plotcfg):
        self.label = label
        self.ts = []
        self.val = []
        self.cfg = plotcfg
        self.bakup_cfg = []

    def add(self, ts, val):
        if len(self.ts) > 0:
            if self.ts[-1] > ts:
                logger.error('timestamp backward, last_ts = %s, ts = %s' % (self.ts[-1], ts))
                return
            elif self.ts[-1] == ts:
                # logger.warn('same timestamp for metric, ts = %s, %s = %s' % (self.ts, self.label, self.val))
                self.ts.pop()
                self.val.pop()
        self.ts.append(ts)
        self.val.append(val)

    def filter(self, filter_func=None):
        if filter_func is None:
            return self

        cfg = deepcopy(self.cfg)
        cls = self.__class__
        filtered = cls.__new__(cls)
        filtered.__init__(self.label, cfg)
        for ts, val in zip(self.ts, self.val):
            if filter_func(ts, val):
                filtered.add(ts, val)

        return filtered

    def backup_cfg(self):
        self.bakup_cfg.append(deepcopy(self.cfg))
        # print 'backup cfg for %s, subplot_idx = %d, bakup_cfg = %s, self = %s' % (self.label, self.cfg.subplot_idx, self.bakup_cfg, self)

    def restore_cfg(self):
        self.cfg = self.bakup_cfg.pop()
        # print 'restore cfg for %s, subplot_idx = %d, bakup_cfg = %s, self = %s' % (self.label, self.cfg.subplot_idx, self.bakup_cfg, self)


class Metrics(RawDataBase):

    def __init__(self, label, plotcfg):
        super(Metrics, self).__init__(label, plotcfg)
        self.threshold = None

    def add_metric(self, ts, val):
        self.add(ts, val)
        # print 'add_metrics %s %s %s' % (self.label, ts, val)

    def set_threshold(self, val):
        self.threshold = val

    def count(self):
        return len(self.val)

    def filter(self, filter_func=None):
        filtered = super(Metrics, self).filter(filter_func)
        filtered.threshold = self.threshold
        return filtered


class Events(RawDataBase):

    def __init__(self, label, plotcfg):
        super(Events, self).__init__(label, plotcfg)

    def add_event(self, ts, meta=None):
        self.add(ts, meta)

    def check_type(self, evt):
        return evt.find(self.label) != -1

    def count(self):
        return len(self.ts)


class RecordFilter(object):

    def __call__(self, r):
        return r


class RecordFilterByDatetime(RecordFilter):

    def __init__(self, start_ts, end_ts):
        self.start_ts = start_ts
        self.end_ts = end_ts

    def __call__(self, r):
        if (self.start_ts is not None and r.ts < self.start_ts) or \
           (self.end_ts is not None and r.ts > self.end_ts):
            return None
        else:
            return r


class LogItem(object):

    # May 12 11:10:08.019 2014
    ios_timestamp_re = re.compile(r'(\w+ \d\d \d\d:\d\d:\d\d\.\d\d\d \d\d\d\d)')
    android_timestamp_re = re.compile(r'(\d\d-\d\d \d\d:\d\d:\d\d\.\d\d\d)')
    # new format 2014-05-14T22:02:21.066
    new_timestamp_re = re.compile(r'(\d\d\d\d-\d\d-\d\d\w\d\d:\d\d:\d\d\.\d\d\d)')
    # TA format 05/16/14 06:21:25.765
    ta_timestamp_re = re.compile(r'(\d\d/\d\d/\d\d \d\d:\d\d:\d\d\.\d\d\d)')
    timestamp_re = [ios_timestamp_re, android_timestamp_re, new_timestamp_re, ta_timestamp_re]

    @classmethod
    def parse(cls, line, filters=[]):
        # check log content first then timestamp
        match = cls.match_re.search(line)
        if match is None:
            return None

        # match, store the metrics/events and then get timestamp
        # split into two steps make it more flexible to changes of log format
        record = map(type_convert, match.groups())
        match = None
        for match_re in LogItem.timestamp_re:
            match = match_re.search(line)
            if match:
                break

        if match is None:
            print 'Error in timestamp format: %s' % line
            return None

        record.extend(map(dt_parser.parse, match.groups()))
        r = cls._make(record)

        for f in filters:
            r = f(r)
            if r is None:
                return None

        return r


class LogParser(object):

    def __init__(self):
        self.view_filter = None
        self.view_trans = None
        self.cached_attrs = {}
        self.view_id = ''

    def set_viewfilter(self, f):
        self.view_filter = f
        self.cached_attrs = {}

    def set_viewtrans(self, t):
        self.view_trans = t

    def set_viewid(self, view_id):
        self.view_id = view_id

    def get_title_with_viewid(self, title):
        return title if self.view_id == '' else '%s %s' % (title, self.view_id)

    def __getattribute__(self, name):
        # print '__getattribute__', name
        item = object.__getattribute__(self, name)
        if isinstance(item, Metrics) or isinstance(item, Events):
            if name in self.cached_attrs:
                # print 'find in cache', name, self.cached_attrs[name]
                return self.cached_attrs[name]
            else:
                cached_obj = item.filter(self.view_filter)
                self.cached_attrs[name] = cached_obj
                # print 'store in cache', name, self.cached_attrs[name]
                if self.view_filter is not None:
                    self.view_trans(cached_obj)
                return cached_obj
        else:
            return item

    def parse(self, line, filters=[]):
        pass

    def analyze(self):
        pass

    def plot(self):
        pass


'''
 Media Statistics for call ID - ab0c42cc-cb93-33dd-8ce9-33dfe658ec4b,2014-05-21T06:00:44.510Z,0c4cc58f-889b-4ea8-9216-3a71f35d9b9d,8
 Media Statistics for call ID - 382ea94e-b6c4-4a39-9ce7-85bb53dbd7f3,2014-05-21T23:23:16.852Z,48F4DEDD-45E6-4B58-80C9-C278DB2AFAC0
 Locus ID, Locus Timestamp in utc, Tracking ID, [comma], time offset, time offset is used convert andriod log timestamp to utc
 This is used to mark the beginning of the current call as well as the end of previous call, and the end of last call is marked by EOF
'''


class AndroidCallInfoLogItem(namedtuple('AndriodCallInfo', 'locusid, locus_ts, trackingid, offset_ts, ts'), LogItem):

    match_re = re.compile(r'Media Statistics for call ID - ([^,]*),([^,]*),([^,]*),([^,\r\n]*)')


class IosCallInfoLogItem(namedtuple('IosCallInfo', 'locusid, locus_ts, trackingid, ts'), LogItem):

    match_re = re.compile(r'Media Statistics for call ID - ([^,]*),([^,]*),([^,\r\n]*)')


class VideoSenderStatsLogItem(namedtuple('VideoSenderStats', 'bytes, pkt, rtt, jitter, loss, rrwin, width, height, fps, br, idrreq, idrsent, ts'), LogItem):

    match_re = re.compile(r'VideoSender - bytes=(\d+) pkt=(\d+) rtt=(\d+) jitter=(\d+) loss=([\.\d]+) rrWin=(\d+) dimensions=(\d+)x(\d+) fps=(\d+) br=(\d+) idrReq=(\d+) idrSent=(\d+)')


class VideoReceiverStatsLogItem(namedtuple('VideoReceiverStats', 'bytes, pkt, jitter, loss, rrwin, ooo, dup, rtpdrop, ooodrop, feclevel, fecrecover, width, height, fps, br, idrreq, idrrecvd, decodedrop, ts'), LogItem):

    match_re = re.compile(r'VideoReceiver - bytes=(\d+) pkt=(\d+) jitter=(\d+) loss=([\.\d]+) rrWin=(\d+) ooo=(\d+) dup=(\d+) rtpDrp=([\.\d]+) oooDrp=([\.\d]+) fecLevel=(\d+) fecRecover=(\d+) dimensions=(\d+)x(\d+) fps=(\d+) br=(\d+) idrReq=(\d+) idrRcvd=(\d+) decodeDrp=([\.\d]+)')


class AudioSenderStatsLogItem(namedtuple('AudioSenderStats', 'bytes, pkt, rtt, jitter, loss, rrWin, br, fecon, ts'), LogItem):

    match_re = re.compile(r'AudioSender - bytes=(\d+) pkt=(\d+) rtt=(\d+) jitter=(\d+) loss=([\.\d]+) rrWin=(\d+) br=(\d+) cFecOn=(\d+)')


class AudioReceiverStatsLogItem(namedtuple('AudioReceiverStats', 'bytes, pkt, jitter, loss, rrwin, ooo, dup, rtpDrp, oooDrop, feclevel, fecrecover, br, jitterdrop, cfecon, cfecrecover, ts'), LogItem):

    match_re = re.compile(r'AudioReceiver - bytes=(\d+) pkt=(\d+) jitter=(\d+) loss=([\.\d]+) rrWin=(\d+) ooo=(\d+) dup=(\d+) rtpDrp=([\.\d]+) oooDrp=([\.\d]+) fecLevel=(\d+) fecRecover=(\d+) br=(\d+) jitterDrp=(\d+) cFecOn=(\d+) cFecRecover=(\d+)')


class CallLogParser(LogParser):

    last_ts = None

    def __init__(self):
        super(CallLogParser, self).__init__()
        self.call_info = Events('CallInfo', PlotCfg())

    def parse(self, line, filters):

        def update_last_call_end(call_info):
            last_end_ts = deepcopy(CallLogParser.last_ts)
            # print '----->set call last end: %s' % last_end_ts
            if len(call_info.val) > 0:
                call_info.val[-1]['end_ts'] = last_end_ts

        r = AndroidCallInfoLogItem.parse(line, filters)
        if r is not None:
            update_last_call_end(self.call_info)
            self.call_info.add_event(r.ts, {'locusid': r.locusid,
                                            'locus_ts': r.locus_ts,
                                            'trackingid': r.trackingid,
                                            'offset_ts': r.offset_ts,
                                            'end_ts': None
                                            })
            # print '----->set call start: %s' % r.ts
            return True

        r = IosCallInfoLogItem.parse(line, filters)
        if r is not None:
            update_last_call_end(self.call_info)
            self.call_info.add_event(r.ts, {'locusid': r.locusid,
                                            'locus_ts': r.locus_ts,
                                            'trackingid': r.trackingid,
                                            'offset_ts': 0,
                                            'end_ts': None
                                            })
            # print '----->set call start: %s' % r.ts
            return True

        r = VideoReceiverStatsLogItem.parse(line, filters)
        if r is not None:
            CallLogParser.last_ts = r.ts
            return True

        r = VideoSenderStatsLogItem.parse(line, filters)
        if r is not None:
            CallLogParser.last_ts = r.ts
            return True

        r = AudioReceiverStatsLogItem.parse(line, filters)
        if r is not None:
            CallLogParser.last_ts = r.ts
            return True

        r = AudioSenderStatsLogItem.parse(line, filters)
        if r is not None:
            CallLogParser.last_ts = r.ts
            return True

        return False


"""
QoS relating metrics
-----------------------

1. QoS Adjustment

2. TMMBR Notification(todo)

3. Bandwidth Allocation(todo)
"""


# seperate old/new so we can remove the outdated one easily later
class OldQoSAdjustmentLogItem(namedtuple('QoSAdjustment', 'state, evalbw, inputrate, sendrate, recvrate, qdelay, rtt, lossrate, convid, ts'), LogItem):

    match_re = re.compile(r'Adjustment, groupKey = [\w|\d]*, state = (\w+\|?\w+), m_dwBandwidth = (\d+), dwInputRate = (\d+), dwOutputRate = (\d+), dwReceiveRate = (\d+), wMaxJitter = (\d+), dwMaxRTT = (\d+), fMaxLossRate = (\d+\.\d+) this=(0[xX][0-9a-fA-F]+)')


class QoSAdjustmentLogItem(namedtuple('OldQoSAdjustment', 'state, evalbw, inputrate, sendrate, recvrate, qdelay, rtt, lossrate, convid, ts'), LogItem):

    match_re = re.compile(r'Adjustment, groupKey = [\w|\d]*, state = (\w+\|?\w+), bandwidth = (\d+), inputRate = (\d+), outputRate = (\d+), receiveRate = (\d+), qdelay = (\d+), rtt = (\d+), lossRatio = (\d+\.\d+) this=(0[xX][0-9a-fA-F]+)')


class QoSAdjustmentLogParser(LogParser):

    def __init__(self):
        super(QoSAdjustmentLogParser, self).__init__()
        self.new_format = True
        self.evalbw = Metrics('evalbw', PlotCfg(0, units='bps'))
        self.evalbw.set_threshold(args.qos_bw_limit)
        self.encoderate = Metrics('inputrate', PlotCfg(0, units='bps'))
        self.sendrate = Metrics('sendrate', PlotCfg(0, units='bps'))
        self.recvrate = Metrics('recvrate', PlotCfg(0, units='bps'))
        self.rtt = Metrics('rtt', PlotCfg(1, units='ms'))
        self.rtt.set_threshold(args.qos_rtt_threshold)
        self.qdelay = Metrics('qdelay', PlotCfg(1, units='ms'))
        self.qdelay.set_threshold(args.qos_qdelay_threshold)
        self.lossrate = Metrics('lossrate', PlotCfg(2))
        self.lossrate.set_threshold(args.qos_loss_threshold)
        self.adjustdown = Events('BANDWIDTH_DOWN', PlotCfg(color=stress_color_cycle.next()))
        self.adjustup = Events('BANDWIDTH_UP', PlotCfg())
        self.newsession = Events('NewSession', PlotCfg())
        self.last_id = None

    def parse(self, line, filters):
        r = QoSAdjustmentLogItem.parse(line, filters)
        if r is not None:
            self.add_record(r)
            return True

        r = OldQoSAdjustmentLogItem.parse(line, filters)
        if r is not None:
            self.new_format = False
            self.add_record(r)
            return True

        return False

    def add_record(self, r):
        if r.convid != self.last_id:
            self.last_id = r.convid
            self.newsession.add_event(r.ts)

        evalbw_bps, inputrate_bps, sendrate_bps, recvrate_bps = r.evalbw, r.inputrate, r.sendrate, r.recvrate
        if not self.new_format:
            rates = [r.evalbw, r.inputrate, r.sendrate, r.recvrate]
            evalbw_bps, inputrate_bps, sendrate_bps, recvrate_bps = map(lambda i: 8*i, rates)

        self.evalbw.add_metric(r.ts, evalbw_bps)
        self.encoderate.add_metric(r.ts, inputrate_bps)
        self.sendrate.add_metric(r.ts, sendrate_bps)
        self.recvrate.add_metric(r.ts, recvrate_bps)
        self.rtt.add_metric(r.ts, r.rtt)
        self.qdelay.add_metric(r.ts, r.qdelay)
        self.lossrate.add_metric(r.ts, r.lossrate)

        if self.adjustdown.check_type(r.state):
            self.adjustdown.add_event(r.ts, r.state)
        elif self.adjustup.check_type(r.state):
            self.adjustup.add_event(r.ts, r.state)

    def plot(self):
        show_metrics_events([self.evalbw, self.sendrate, self.recvrate, self.rtt, self.qdelay, self.lossrate],
                            [self.adjustdown, self.newsession],
                            self.get_title_with_viewid('QoSAdjustment'))
        show_metrics_hist([self.rtt, self.lossrate, self.qdelay, self.sendrate, self.recvrate, self.evalbw],
                          self.get_title_with_viewid('QoSHist'))


"""
Video relating metrics
-----------------------

1. capture spatial/temporal

2. encode spatial/temporal

3. decode spatial/temporal
"""


class VideoCaptureLogItem(namedtuple('VideoCapture', 'width, height, temporal, ts'), LogItem):

    match_re = re.compile('CWseH264SvcEncoder::UpdateEncodeParam\(\) capture param: \d+, (\d+) x (\d+) @ (\d+\.\d+), source type = \d+')


class VideoEncodeLogItem(namedtuple('VideoEncode', 'width, height, temporal, ts'), LogItem):

    match_re = re.compile(r'CWseH264SvcEncoder::UpdateEncodeParam\(\) spatial \d+\: width=(\d+), height=(\d+), frame_rate=(\d+\.\d+)')


class VideoDecodeSpatialLogItem(namedtuple('VideoDecode', 'width, height, id, ts'), LogItem):

    match_re = re.compile(r'CWseVideoListenChannel::OnDecoded, src_id=\d+,width=(\d+), height=(\d+),len=\d+,iColorSpace=\d+,this=(0[xX][0-9a-fA-F]+)')


class VideoDecodeTemporalLogItem(namedtuple('VideoDecode', 'temporal, id, ts'), LogItem):

    match_re = re.compile(r'CWseVideoListenChannel::OnDecoded,Render Fps:(\d+\.\d+),src_id=\d+,this=(0[xX][0-9a-fA-F]+)')


class VideoPLIRequestLogItem(namedtuple('VideoPLIRequest', 'ts'), LogItem):

    match_re = re.compile(r'CRTPSessionClientVideo:: OnPictureLossIndication: PLI request, SSRC = \d+, channel id = \d+, stream id = \d+')


class VideoForceKeyFrameLogItem(namedtuple('VideoForceKeyFrame', 'ts'), LogItem):

    match_re = re.compile(r'CWseVideoSourceChannel::ForceKeyFrame, m_uIDRReqNum=\d+')


class VideoOpenDeviceFailedLogItem(namedtuple('VideoOpenDeviceFailed', 'ts'), LogItem):

    match_re = re.compile(r'init, open Camera fail, deviceID|CWseIPHoneVideoCapSession initWithDevcieName init device failed')


class VideoDecodeFailedLogItem(namedtuple('VideoDecodeFailed', 'ts'), LogItem):

    match_re = re.compile(r'CWseH264SvcDecoder::DecodeFrame failed')


class VideoLogParser(LogParser):

    def __init__(self):
        super(VideoLogParser, self).__init__()
        yticks = [0, 90, 180, 360, 720]
        self.capture_resolution = Metrics('capture resolution', PlotCfg(0, linestyle='steps-post', yticks=yticks))
        self.capture_framerate = Metrics('capture framerate', PlotCfg(1, units='bps'))
        self.encode_resolution = Metrics('encode resolution', PlotCfg(0, linestyle='steps-post', yticks=yticks))
        self.encode_framerate = Metrics('encode framerate', PlotCfg(1, units='bps'))
        self.decode_resoluton = Metrics('decode resolution', PlotCfg(0, linestyle='steps-post', yticks=yticks))
        self.decode_framerate = Metrics('decode framerate', PlotCfg(1, units='bps'))
        self.pli_request = Events('PLI request', PlotCfg())
        self.force_keyframe = Events('force keyframe', PlotCfg())
        self.abnormal_pli_request = Events('Abnormal PLI request', PlotCfg(color=stress_color_cycle.next()))
        self.abnormal_force_keyframe = Events('Abnormal Force keyframe', PlotCfg(color=stress_color_cycle.next()))
        self.device_failed = Events('Open Device failed', PlotCfg(color=stress_color_cycle.next()))
        self.decode_failed = Events('Decode failed', PlotCfg(color=stress_color_cycle.next()))

    def parse(self, line, filters):
        r = VideoCaptureLogItem.parse(line, filters)
        if r is not None:
            self.capture_resolution.add_metric(r.ts, min(r.width, r.height))
            self.capture_framerate.add_metric(r.ts, r.temporal)
            return True

        r = VideoEncodeLogItem.parse(line, filters)
        if r is not None:
            self.encode_resolution.add_metric(r.ts, min(r.width, r.height))
            self.encode_framerate.add_metric(r.ts, r.temporal)
            return True

        r = VideoDecodeSpatialLogItem.parse(line, filters)
        if r is not None:
            self.decode_resoluton.add_metric(r.ts, min(r.width, r.height))
            return True

        r = VideoDecodeTemporalLogItem.parse(line, filters)
        if r is not None:
            self.decode_framerate.add_metric(r.ts, r.temporal)
            return True

        r = VideoPLIRequestLogItem.parse(line, filters)
        if r is not None:
            self.pli_request.add_event(r.ts)
            return True

        r = VideoForceKeyFrameLogItem.parse(line, filters)
        if r is not None:
            self.force_keyframe.add_event(r.ts)
            return True

        r = VideoOpenDeviceFailedLogItem.parse(line, filters)
        if r is not None:
            self.device_failed.add_event(r.ts)
            return True

        r = VideoDecodeFailedLogItem.parse(line, filters)
        if r is not None:
            self.decode_failed.add_event(r.ts)

        return False

    def analyze(self):
        log_header1('Video analyzing...')
        self.analyze_no_video()
        self.analyze_pli_keyframe()

    def analyze_no_video(self):
        log_header2('Analyzing no video issue...')

        log_header3('Checking device status...')
        for ts in self.device_failed.ts:
            logger.error('Open camera failed at %s' % ts)
        if self.device_failed.count() == 0:
            logger.info('Device OK')

    def analyze_pli_keyframe(self):
        log_header2('Analyzing PLI request and enforce key frame...')
        self.analyze_pli()
        self.analyze_keyframe()

        if self.pli_request.count() != self.force_keyframe.count():
            logger.error('\n**Received PLI request count (%d) not matching with force key frame sent count (%d)**'
                         % (self.pli_request.count(), self.force_keyframe.count()))

    def analyze_pli(self):
        if self.pli_request.count() == 0:
            logger.info('\nNo PLI request received')
            return

        pli_request_array = np.array([dt_to_epoch_ms(ts) for ts in self.pli_request.ts])
        pli_request_intervals = np.diff(pli_request_array)
        logger.debug('\nPLI request interval (less than 500ms will be marked with ###) :')
        logger.debug('-'*80+'\n')
        for idx, i in enumerate(pli_request_intervals):
            mark = ''
            if i < 500:
                mark = '###'
                self.abnormal_pli_request.add_event(self.pli_request.ts[idx])
            logger.debug('%s\t%s\t%i(ms) %s' % (self.pli_request.ts[idx], self.pli_request.ts[idx+1], i, mark))

        logger.info('\nPLI statistics:')
        if len(pli_request_intervals) == 0:
            logger.info('Only one PLI request, no interval calculated')
            return

        logger.info('Video PLI request count: %d, avg_interval: %d ms, min_interval: %d ms, max_interval: %d ms'
                    % (len(pli_request_array), np.average(pli_request_intervals), pli_request_intervals.min(), pli_request_intervals.max()))

    def analyze_keyframe(self):
        if self.force_keyframe.count() == 0:
            logger.info('\nNo force keyframe sent')
            return

        force_keyframe_array = np.array([dt_to_epoch_ms(ts) for ts in self.force_keyframe.ts])
        force_keyframe_intervals = np.diff(force_keyframe_array)
        logger.debug('\nFroce keyframe interval (less than 500ms will be marked with ###) :')
        logger.debug('-'*80+'\n')
        for idx, i in enumerate(force_keyframe_intervals):
            mark = ''
            if i < 500:
                mark = '###'
                self.abnormal_force_keyframe.add_event(self.force_keyframe.ts[idx])
            logger.debug('%s\t%s\t%i(ms) %s' % (self.force_keyframe.ts[idx], self.force_keyframe.ts[idx+1], i, mark))

        logger.info('\nForce keyframe statistics:')
        if len(force_keyframe_intervals) == 0:
            logger.info('Only one Force key frame, no interval calculated')
            return
        logger.info('Video force keyframe count: %d, avg_interval: %d ms, min_interval: %d ms, max_interval: %d ms'
                    % (len(force_keyframe_array), np.average(force_keyframe_intervals), force_keyframe_intervals.min(), force_keyframe_intervals.max()))

    def plot(self):
        events_lst = [self.abnormal_force_keyframe,
                      self.abnormal_pli_request,
                      self.device_failed,
                      self.decode_failed
                      ]
        show_metrics_events([
            self.capture_resolution, self.capture_framerate,
            self.encode_resolution, self.encode_framerate,
            self.decode_resoluton, self.decode_framerate
            ],
            events_lst,
            self.get_title_with_viewid('Video Metrics'))


"""
Audio relating metrics
-----------------------

1. capture frame count

2. jitter buffer min/max/avg size and avg delay
"""


class AudioCaptureLogItem(namedtuple('AudioCapture', 'captured_frames, id, ts'), LogItem):

    match_re = re.compile(r'event test in OnData getevent_num:(\d+),this=(0[xX][0-9a-fA-F]+)')


class AudioJitterBufferLogItem(namedtuple('AudioJitterBuffer', 'max_bufsize, min_bufsize, avg_bufsize, avg_delay, id, ts'), LogItem):

    match_re = re.compile(r'CAudioJitterBuffer::OnTimer, BuffIno, ssrc :\d+, max BS:(\d+), min BS:(\d+), Average BS:(\d+), \w+, m_ulAverageDelay:(\d+),this=(0[xX][0-9a-fA-F]+)')


class AudioJitterBufferDropLogItem(namedtuple('AudioJitterBufferDrop', 'ts'), LogItem):

    match_re = re.compile(r'CAudioJitterBuffer::RemoveFirstPacket')


class AudioLogParser(LogParser):

    def __init__(self):
        super(AudioLogParser, self).__init__()
        self.max_bufsize = Metrics('Max jitter buffer size', PlotCfg(0, units='pkts'))
        self.min_bufsize = Metrics('Min jitter buffer size', PlotCfg(0, units='pkts'))
        self.avg_bufsize = Metrics('Avg jitter buffer size', PlotCfg(0, units='pkts'))
        self.avg_delay = Metrics('Avg jitter buffer delay', PlotCfg(1, units='ms'))
        self.captured_frames = Metrics('captured frames', PlotCfg(2))
        self.jitterbuffer_drops = Events('Jitter buffer drops', PlotCfg(color=stress_color_cycle.next()))

    def parse(self, line, filters):
        r = AudioCaptureLogItem.parse(line, filters)
        if r is not None:
            self.captured_frames.add_metric(r.ts, r.captured_frames)
            return True

        r = AudioJitterBufferLogItem.parse(line, filters)
        if r is not None:
            self.max_bufsize.add_metric(r.ts, r.max_bufsize)
            self.min_bufsize.add_metric(r.ts, r.min_bufsize)
            self.avg_bufsize.add_metric(r.ts, r.avg_bufsize)
            self.avg_delay.add_metric(r.ts, r.avg_delay)
            return True

        r = AudioJitterBufferDropLogItem.parse(line, filters)
        if r is not None:
            self.jitterbuffer_drops.add_event(r.ts)
            return True

        return False

    def plot(self):
        show_metrics_events([self.captured_frames, self.max_bufsize,
                             self.min_bufsize, self.avg_bufsize, self.avg_delay],
                            [self.jitterbuffer_drops],
                            self.get_title_with_viewid('Audio Metrics'))
        show_metrics_hist([self.max_bufsize, self.min_bufsize, self.avg_bufsize, self.avg_delay],
                          self.get_title_with_viewid('Audio Metrics Histogram'))


'''
plot functions
----------------

1. plot metrics distribution using histogram

2. plot time serials of metrics/events
'''


def show_metrics_hist(metrics, title, numcols=2, binsize=50):
    metrics = [m for m in metrics if m.count() != 0]
    if len(metrics) == 0:
        return

    fig = plt.figure()
    fig.suptitle(title)
    plt.subplots_adjust(hspace=0.5)
    numrows = (len(metrics)+numcols-1) / numcols
    for idx, m in enumerate(metrics):
        if m.count() == 0:
            continue
        ax = fig.add_subplot(numrows, numcols, idx+1)
        ax.xaxis.get_major_formatter().set_useOffset(False)
        ax.set_xlabel(m.label + '(%s)' % m.cfg.units)
        ax.set_ylabel('Probability')
        weights = np.repeat(1.0/len(m.val), len(m.val))
        ax.hist(m.val, binsize, weights=weights)

    if args.savepng is not None:
        plt.savefig(args.savepng + os.sep + "%s.png" % title)
    else:
        plt.show()


def reassign_subplot_idx(metrics):
    sorted_idxes = list(set([m.cfg.subplot_idx for m in metrics]))
    for m in metrics:
        m.cfg.subplot_idx = sorted_idxes.index(m.cfg.subplot_idx)


def show_metrics_events(metrics, events, title):
    metrics = [m for m in metrics if m.count() > 1]
    if len(metrics) == 0:
        return

    [m.backup_cfg() for m in metrics]
    reassign_subplot_idx(metrics)

    subplot_cnt = max([m.cfg.subplot_idx for m in metrics])+1
    numcols = 1
    numrows = (subplot_cnt+numcols-1) / numcols
    fig, axes = plt.subplots(numrows, numcols, sharex=True)
    if isinstance(axes, mpl.axes.SubplotBase):
        axes = [axes]

    fig.suptitle(title)
    fig.subplots_adjust(hspace=0.4)
    ylabels = {}
    for m in metrics:
        axes[m.cfg.subplot_idx].plot(m.ts, m.val, label=m.label, color=m.cfg.color, linestyle=m.cfg.linestyle, marker='.')
        ylabels.setdefault(m.cfg.subplot_idx, '')
        units = ''
        if m.cfg.units is not '':
            units = '(' + m.cfg.units + ')'
        ylabels[m.cfg.subplot_idx] = ylabels[m.cfg.subplot_idx] + m.label + units + '\n'
        if m.cfg.xticks is not None:
            axes[m.cfg.subplot_idx].set_xticks(m.cfg.xticks)
        if m.cfg.yticks is not None:
            axes[m.cfg.subplot_idx].set_yticks(m.cfg.yticks)

        if m.threshold is not None:
            axes[m.cfg.subplot_idx].axhline(y=m.threshold, color=m.cfg.color, linestyle='-', linewidth=1, label='%s_threshold' % m.label)

    for ax in axes:
        bottom, top = ax.get_ylim()
        ax.set_ylim(bottom, 1.05*top)
        ax.xaxis.set_minor_locator(dates.SecondLocator())

    for e in events:
        for ts in e.ts:
            for ax in axes:
                ax.axvline(x=ts, color=e.cfg.color, linestyle='-.', linewidth=2, label=e.label)

    if not args.nogrid:
        [ax.grid(True) for ax in axes]

    for idx, label in ylabels.iteritems():
        lines = []
        labels = []
        axes[idx].set_ylabel(label)
        line, label = axes[idx].get_legend_handles_labels()
        lines = lines + line
        labels = labels + label
        by_label = OrderedDict(zip(labels, lines))
        axes[idx].legend(by_label.values(), by_label.keys(), bbox_to_anchor=(0., 1.02, 1., .102),
                         loc="lower left", ncol=len(labels), mode='expand', borderaxespad=0.)

    if args.savepng is not None:
        plt.savefig(args.savepng + os.sep + "%s.png" % title)
    else:
        plt.show()

    [m.restore_cfg() for m in metrics]


def plot_audio_qos(audio_parser, qos_parser):
    title = 'audio-qos metrics'
    metrics = [audio_parser.avg_delay,
               qos_parser.rtt, qos_parser.qdelay,
               audio_parser.max_bufsize, audio_parser.min_bufsize,
               audio_parser.avg_bufsize,
               qos_parser.lossrate]
    [m.backup_cfg() for m in metrics]

    audio_parser.avg_delay.cfg.subplot_idx = 0
    qos_parser.rtt.cfg.subplot_idx = 0
    qos_parser.qdelay.cfg.subplot_idx = 0
    qos_parser.lossrate.cfg.subplot_idx = 1
    audio_parser.max_bufsize.cfg.subplot_idx = 2
    audio_parser.min_bufsize.cfg.subplot_idx = 2
    audio_parser.avg_bufsize.cfg.subplot_idx = 2
    events = [qos_parser.adjustdown, qos_parser.newsession]

    show_metrics_events(metrics, events, audio_parser.get_title_with_viewid(title))

    [m.restore_cfg() for m in metrics]


def plot_video_qos(video_parser, qos_parser):
    title = 'video-qos metrics'
    metrics = [qos_parser.rtt, qos_parser.qdelay, qos_parser.lossrate,
               qos_parser.evalbw, qos_parser.sendrate, qos_parser.recvrate,
               video_parser.capture_resolution, video_parser.encode_resolution]
    [m.backup_cfg() for m in metrics]

    qos_parser.rtt.cfg.subplot_idx = 0
    qos_parser.qdelay.cfg.subplot_idx = 0
    qos_parser.lossrate.cfg.subplot_idx = 1
    qos_parser.evalbw.cfg.subplot_idx = 2
    qos_parser.sendrate.cfg.subplot_idx = 2
    qos_parser.recvrate.cfg.subplot_idx = 2
    video_parser.capture_resolution.cfg.subplot_idx = 3
    video_parser.encode_resolution.cfg.subplot_idx = 3

    events = [video_parser.decode_failed,
              # video_parser.pli_request,
              qos_parser.newsession,
              qos_parser.adjustdown]

    show_metrics_events(metrics, events, video_parser.get_title_with_viewid(title))

    [m.restore_cfg() for m in metrics]


def analyze_and_plot(parsers, start_ts=None, end_ts=None, view_id='', offset=0):
    for k, p in parsers.iteritems():
        if k != 'callinfo':
            p.set_viewfilter(MetricsEventsFilterByDatetime(start_ts, end_ts))
            p.set_viewid(view_id)
            p.set_viewtrans(MetricsEventsTimestampTransformer(offset))
        p.analyze()
        p.plot()

    if args.audio_qos:
        plot_audio_qos(parsers['audio'], parsers['qos'])

    if args.video_qos:
        plot_video_qos(parsers['video'], parsers['qos'])

    if args.savepng is not None:
        print '\nsave files to %s' % args.savepng
        shutil.move(LOG_FILE_PATH, args.savepng + os.sep + os.path.basename(LOG_FILE_PATH))


if __name__ == '__main__':

    argParser = argparse.ArgumentParser()
    argParser.add_argument("logpath", help="log path or log directory path")
    argParser.add_argument("--nogrid", action="store_true", default=False, help="Disable grids in plots")
    argParser.add_argument("--utc", action="store_true", default=False, help="Convert timestamp to utc if possible")
    argParser.add_argument("--savepng", nargs='?', default='', help="Save plots to files, default to the directory where the logdir|logfile is")
    argParser.add_argument("--qos", action="store_true", default=False, help="Show qos metrics")
    argParser.add_argument("--audio", action="store_true", default=False, help="Show audio metrics")
    argParser.add_argument("--video", action="store_true", default=False, help="Show video metrics")
    argParser.add_argument("--audio-qos", action="store_true", default=False, help="Show audio and qos metrics in one graph as subplots")
    argParser.add_argument("--video-qos", action="store_true", default=False, help="Show video and qos metrics in one graph as subplots")
    argParser.add_argument("--all", action="store_true", default=True, help="Show all metrics, default")
    argParser.add_argument("--start-ts", help="Set the start timestamp(local timezone) string (unix epoch not support) for filter the metrics")
    argParser.add_argument("--end-ts", help="Set the end timestamp(local timezone) string (unix epoch not support) for filter the metrics")
    argParser.add_argument("--qos-bw-limit", help="Show horizontal line as the bandwidth limitation")
    argParser.add_argument("--qos-loss-threshold", help="Show horizontal line as loss threshold value")
    argParser.add_argument("--qos-qdelay-threshold", help="Show horizontal line as queuing delay threshold value")
    argParser.add_argument("--qos-rtt-threshold", help="Show horizontal line as rtt threshold value")

    args = argParser.parse_args()
    # print args

    # set the save path the same as the log file if the path is not specified when savepng option exists
    # if no "--savepng", args.savepng is '' by default, and if "--savepng' with no param, args.savepng is None
    if args.savepng is '':
        args.savepng = None
    elif args.savepng is None:
        args.savepng = os.path.dirname(args.logpath)
    else:
        args.savepng = os.path.abspath(args.savepng)

    logfiles = []
    if os.path.isdir(args.logpath):
        for f in glob.glob(args.logpath+os.sep+'*.txt'):
            logfiles.append(f)
    elif os.path.isfile(args.logpath):
        logfiles.append(args.logpath)
    else:
        argParser.error('Invalid logpath %s' % args.logpath)

    # sort files by date thus we get the metrics/events by timestamp
    logfiles.sort(key=lambda x: os.stat(x).st_mtime)

    mpl.rcParams['figure.figsize'] = 18, 9.5

    if args.audio_qos:
        args.audio = args.qos = True

    if args.video_qos:
        args.video = args.qos = True

    if any([args.qos, args.audio, args.video]):
        # show only specified metrics
        args.all = False

    if args.all:
        args.qos = args.audio = args.video = args.audio_qos = args.video_qos = True

    parsers = {}
    parsers['callinfo'] = CallLogParser()
    if args.qos:
        parsers['qos'] = QoSAdjustmentLogParser()
    if args.audio:
        parsers['audio'] = AudioLogParser()
    if args.video:
        parsers['video'] = VideoLogParser()

    try:
        if args.start_ts is not None:
            args.start_ts = dt_parser.parse(args.start_ts)
        if args.end_ts is not None:
            args.end_ts = dt_parser.parse(args.end_ts)
    except ValueError, e:
        argParser.error('Error in datetime format: %s' % e)

    filter_ts = RecordFilterByDatetime(args.start_ts, args.end_ts)
    filters = [filter_ts]

    logger.debug('\nparsing log file...')
    for filepath in logfiles:
        print filepath
        f = open(filepath, 'r')
        for l in f.readlines():
            for p in parsers.values():
                ret = p.parse(l, filters)
                if ret:
                    break

        f.close()

    for idx, (start_ts, meta) in enumerate(zip(parsers['callinfo'].call_info.ts, parsers['callinfo'].call_info.val)):
        end_ts = meta['end_ts']
        utc_start_ts = convert_to_utc(start_ts, meta['offset_ts'])
        utc_end_ts = None if end_ts is None else convert_to_utc(end_ts, meta['offset_ts'])
        offset = meta['offset_ts'] if args.utc else 0
        call_id = 'call_%d_locusid_%s' % (idx, meta['locusid'])
        log_header0('call %d \nlocal time: [%s - %s] \nutc time: [%s - %s] \nmeta: %s' %
                    (idx, start_ts, end_ts, utc_start_ts, utc_end_ts, meta))
        analyze_and_plot(parsers, start_ts, end_ts, call_id, offset)
        if end_ts is None:
            break
    else:
        # when no client log
        analyze_and_plot(parsers)
