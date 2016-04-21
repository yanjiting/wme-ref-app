package com.wx2.log;

import android.content.Context;
//import android.net.Uri;
//import android.os.Environment;
import android.util.Log;

//import com.wx2.util.TestUtils;
//import com.wx2.util.ZipUtils;


import java.io.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

//import javax.inject.Inject;
//import javax.inject.Singleton;

/**
 * This class provides file-based logging for the Ln.Print object.  It acts as a FileProvider so that
 * log files from the apps secure, internal storage location, can be shared with mail apps for sending
 * as attachments.
 *
 * Internal FSLP creates and writes logs to a rolling set of files in the apps 'files/logs' storage.
 * The number and size of files is currently hard-coded below and start at zero (e.g. files/logs/wx2log0.txt)
 * If the logs/ zipDirectory does not exist, it is created.  If an available log index does not exist
 * (wx2log[0-N].txt) it will be created and used.  If the whole range of log files exist, then the
 * oldest file will be found and logging and rotation will commence from there.
 *
 * TODO: The file rolling functionality is home-grown.  Java offers a more full-featured implementation
 * that could/should be investigated.
 *
 * FSLP implements the println() method of Print to handle logging requests and write them to the
 * current log file.
 *
 * Externally, FSLP acts as a FileProvider when logs need to be shared to other apps.  The AndroidManifest.xml
 * defines a (FileProvider) <provider>.  NOTE: The authorities parameter must match LOG_FILE_PROVIDER.
 * That block also points to the log_files.xml file, which provides URI indirection to the storage
 * location.  This XML indicates that our ZIP_FILE_NAME will can be referenced by:
 *  X,
 * while it is stored in:
 *  Y
 *
 * Finally, an external ZipUtils class was created to compress all the files in the logs/ zipDirectory into
 * one single ZIP file.  When getZippedLogsUri() is called, a recursive zip of logs/ is done to produce
 * one file in the files/ zipDirectory.  This is referenced by getUriForFile() to generate a URI another
 * app can use to access the FileProvider and retrieve the file.
 *
 * NOTE: Be aware that changing the LOG_FILE_PROVIDER will require corresponding changes to these
 * XML files.
 */
//@Singleton
//public class LogFilePrint extends LnImpl implements Runnable {
public class LogFilePrint implements Runnable {
    private static final String TAG = "LogFilePrint";
    private static final String LOG_DIR = "logs";
    private static final String LOG_FILE_NAME = "wx2log";
    private static final String LOG_FILE_EXT = ".txt";
//    private static final String ZIP_FILE_NAME = "wx2logs.zip";
//    private static final int  MAX_DEBUG_LOG_FILES = 5;
    private static final int  MAX_RELEASE_LOG_FILES = 5;
    private static final long MAX_LOG_FILE_SIZE = 5 * 1024 * 1024; // 2MB file size

//    private Context context;
    private Process logcatProcess;
    private File logFile;
    private String rootLogDir;
    private int maxLogFiles;
    private int currentLogFileSize;
    private AtomicBoolean running;
    private Thread captureThread;
    // For test automation
    private AtomicReference<String> watchForString = new AtomicReference<String>("");
    private AtomicBoolean watchForStringFound = new AtomicBoolean(false);


    public LogFilePrint() {
        // do nothing, used by Ln before injection is set up
    }

 //   @Inject
    public LogFilePrint(Context context) {
//        super(context);
//        this.context = context;
        this.running = new AtomicBoolean(false);

        // The upper range of log files MUST be set before getNewLogFile() is called.
//        if (TestUtils.isDebug(context)) {
//            maxLogFiles = MAX_DEBUG_LOG_FILES;
//            rootLogDir = context.getExternalCacheDir().toString();
//        } else {
            maxLogFiles = MAX_RELEASE_LOG_FILES;
            rootLogDir = context.getExternalCacheDir().toString();            
//        }
    }

//    @Override
//    public synchronized int println(int priority, String msg) {
//        // Write message to logcat (stdout)
//        return super.println(priority, msg);
 //   }

    public void startCapture() {
        if (!running.get()) {
            Log.d(TAG, "Starting log capture");
            captureThread = new Thread(this, "Log print");
            captureThread.start();
        }
    }

    public void stopCapture() {
        if (running.get()) {
            running.set(false);
            // Send a log message to break out of any blocked input reads.
            Log.i("stopCapture", "Logging stopped");
            stopCaptureThread();
            stopLogcat();
        }
    }

    private void stopCaptureThread() {
        if (captureThread != null) {
            if (captureThread.isAlive()) {
                // This probably doesn't do anything since our io mechanism for logging capture is not interruptible!
                captureThread.interrupt();
            }
            captureThread = null;
        }
    }

    private void stopLogcat() {
        if (logcatProcess != null) {
            logcatProcess.destroy();
            logcatProcess = null;
        }
    }

 /*   public Uri generateZippedLogs() {
        File zipFile = getZipFile();
        if (zipFile != null) {
            // Zip the log directory contents to the specified file
            ZipUtils.zipDirectory(getLogDirectory(), getZipFile());
            return Uri.fromFile(getZipFile());
        } else {
            return null;
        }
    }

    public File getZipFile() {
        File cacheDir = context.getExternalCacheDir();

        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            return new File(cacheDir, ZIP_FILE_NAME);
        } else {
            return null;
        }
    }
*/
    public File getCurrentLogFile() {
        return logFile;
    }

    public File[] getAllLogFiles() {
        return getLogDirectory().listFiles();
    }

    private void getNewLogFile() {
        File logDir = getLogDirectory();
        if (!logDir.exists()) {
            if (!logDir.mkdirs())
                Log.e(TAG, "Failed to make directory");
        }

        rotateLogFiles();
        logFile = buildLogFile(0);
        if (!logFile.exists()) {
            try {
                logFile.createNewFile();
            } catch (IOException e) {
                Log.e(TAG, "Failed to create log file: " + logFile.getAbsolutePath(), e);
            }
        }
        currentLogFileSize = 0;
    }

    private void rotateLogFiles() {
        // File '0' is always most current, so we rotate (rename) files down in numbers
        // until the max is reached.  At which point, we delete the tail/oldest.
        int startIndex = maxLogFiles - 1;
        for (int i = startIndex; i > 0; i--) {
            File fromFile = buildLogFile(i-1);
            File toFile   = buildLogFile(i);
            if (toFile.exists() && (i == startIndex)) {
                if (!toFile.delete())
                    Log.e(TAG, "Failed to delete 'to' file");
            }
            if (fromFile.exists()) {
                if (!fromFile.renameTo(toFile))
                    Log.e(TAG, "Failed to rename file");
            }
        }
    }

    private File buildLogFile(int i) {
        return new File(getLogDirectoryName() + LOG_FILE_NAME + Integer.toString(i) + LOG_FILE_EXT);
    }

    private String getLogDirectoryName() {
        return (rootLogDir + File.separator + LOG_DIR + File.separator);
    }

    public File getLogDirectory() {
        File logDir = new File(getLogDirectoryName());
        if (!logDir.exists()) {
            if (!logDir.mkdirs())
                Log.e(TAG, "Failed to make directory");
        }
        return logDir;
    }

    public void run() {
        capture();
    }

    private void cleanupLogcat() {
        BufferedReader reader = null;
        try {
            // Query process list, which returns:
            //   USER      PID   PPID  VSIZE  RSS     WCHAN    PC         NAME
            //   u0_a141   7716  1     884    180   ffffffff 00000000 S logcat
            //   u0_a141   8028  1     884    180   ffffffff 00000000 S logcat
            //   u0_a141   8211  8189  884    180   ffffffff 00000000 S logcat
            // Orphaned processes have a parent (PPID) of 1
            Process ps = new ProcessBuilder("ps", "logcat").start();
            int exitCode = ps.waitFor();
            if (exitCode > 0)
                Log.w("cleanupLogcat", "ps command failed to list logcat instances (exit code="+exitCode+")");
            reader = new BufferedReader(new InputStreamReader(ps.getInputStream()));
            String line;
            while ((reader != null) && ((line = reader.readLine()) != null)) {
                if (line.endsWith("logcat")) {
                    String[] element = line.split("\\s+");
                    if (element.length == 9 && element[2].equals("1")) {
                        int pid = Integer.parseInt(element[1]);
                        Log.d("cleanupLogcat", "Removing orphaned logcat process ID: "+pid);
                        // NOTE: We only have permission to kill processes we created
                        android.os.Process.killProcess(pid);
                    }
                }
            }
        } catch (IOException ex) {
            Log.w("Encountered an IOException while cleaning up orphaned logcats: %s", ex.getMessage());
        } catch (InterruptedException ex) {
            Log.w("The process searching for orphaned logcats was interrupted: %s", ex.getMessage());
        } finally {
            try {
                if (reader != null)
                    reader.close();
            } catch (IOException ex) {
                Log.w("Encountered an IOException closing the orphaned logcat reader: %s", ex.getMessage());
            }
        }
    }

    private void capture() {
        Log.i("LogFilePrint", "capture start");
        running.set(true);
        try {
            // Remove any orphaned logcat instances before starting a new one.
            cleanupLogcat();

            // Then, clear out logcat prior to now
            String[] clearLogcat = { "logcat", "-c" };
            new ProcessBuilder()
                    .command(clearLogcat)
                    .start()
                    .waitFor();

            // Then, start logcat streaming
            String[] streamLogcat = { "logcat", "-v", "threadtime" };
            logcatProcess = new ProcessBuilder()
                    .command(streamLogcat)
                    .redirectErrorStream(true)
                    .start();

            BufferedReader reader = new BufferedReader(new InputStreamReader(logcatProcess.getInputStream()));
            try {
                String inputLine;
                while (running.get()) {
                    getNewLogFile();
                    FileOutputStream writer = new FileOutputStream(logFile);
                    Log.v(TAG, "Opening log file: " + logFile.getPath());
                    try {
                        boolean fileNotFull = true;
                        while (fileNotFull) {
                            inputLine = reader.readLine();
                            if (inputLine != null) {
                                if (inputLine.contains("Unable to open log device '/dev/log/main': Permission denied")) {
                                    // This means we don't have the permissions to read from logcat.
                                    // It typically happens on custom roms such as Cyanogenmod.
                                    // Not much we can do in this scenario so kill the logcat process and break out of the loop.
                                    stopLogcat();
                                    running.set(false);
                                    break;
                                }
                                writer.write(inputLine.getBytes());
                                writer.write('\n');
                                currentLogFileSize += inputLine.length();
                                if (currentLogFileSize > MAX_LOG_FILE_SIZE) {
                                    Log.v(TAG, "Log file has reached max size. Closing and rotating files now.");
                                    fileNotFull = false;
                                }

                                /*if (TestUtils.isDebug(context)) {
                                    // (For test automation) watch for a string appearance in the input stream
                                    writer.flush();
                                    String test = getWatchForString();
                                    if (!test.isEmpty()) {
                                        if (inputLine.contains(test))
                                            watchForStringFound.set(true);
                                    }
                                }*/
                            } else {
                                Log.w(TAG, "Input from log stream was null.");
                            }
                            if (!running.get()) {
                                break;
                            }
                        }
                    } catch (IOException e) {
                        Log.d(TAG, "Error during read/write of logs", e);
                    } finally {
                        try {
                            writer.flush();
                            writer.close();
                        } catch (IOException e) {
                            Log.d(TAG, "Error closing log writer", e);
                        }
                    }
                }
            } finally {
                reader.close();
            }
        } catch (IOException e) {
            Log.e(TAG, "Error capturing logcat output:", e);
        } catch (InterruptedException e) {
            // Capture stopped.
        }
        Log.v(TAG, "Log capture terminated");
    }


    // The following methods exist for automated testing...
    public void watchFor(String msg) {
        Log.i(TAG, "Setting 'log watch' string");
        setWatchForString(msg);
    }

    public boolean watchForWait(int maxWaitMsecs) {
        Log.i(TAG, "Waiting " + Integer.toString(maxWaitMsecs) + " msecs for 'log watch' string to appear");
        int waitCount = 10;
        int waitInterval = maxWaitMsecs / waitCount;
        boolean found = false;

        while (!found && waitCount > 0) {
            Log.i(TAG, "Check " + Integer.toString(waitCount) + " for 'log watch' string");
            try {
                Thread.sleep(waitInterval);
                waitCount--;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            found = watchForStringFound.get();
        }

        // Reset variables
        setWatchForString("");
        watchForStringFound.set(false);

        Log.i(TAG, "'Log watch' string was " + (found ? "" : "not ") + "found");
        return found;
    }

    private void setWatchForString(String message) {
        watchForString.set((message == null) ? "" : message);
    }

 //   private String getWatchForString() {
 //       return watchForString.get();
 //   }
}
