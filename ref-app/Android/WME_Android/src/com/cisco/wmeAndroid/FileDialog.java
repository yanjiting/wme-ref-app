package com.cisco.wmeAndroid;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.TreeMap;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.graphics.Color;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;


public class FileDialog extends ListActivity {
	private static final String ITEM_NAME = "name";
	private static final String ITEM_LOGO = "logo";
	private static final String ROOT = "/";
	
	public static final String ROOT_PATH = "ROOT_PATH";	
	public static final String FILE_SUFFIX = "FILE_SUFFIX";	
	public static final String FILE_MODE = "FILE_MODE";
	public static final String SUPPORT_DIR = "SUPPORT_DIR";
	public static final String CHOSEN_PATH = "CHOSEN_PATH";

	private List<String> mPathList = null;
	private ArrayList<HashMap<String, Object>> mFileList;
	private HashMap<String, Integer> mLastPositions = new HashMap<String, Integer>();
	
	private TextView mPathView;
	private Button mSelectBtn;
	
	private String mParentPath;
	private String mCurrPath = ROOT;
	private String[] mSuffix = null;
	private boolean mSupportDir = false;
	private File mChosenFile;
	

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setResult(RESULT_CANCELED, getIntent());

		setContentView(R.layout.localfilemain);			
		mPathView = (TextView) findViewById(R.id.path);		
		mSelectBtn = (Button) findViewById(R.id.fdButtonSelect);
		mSelectBtn.setEnabled(false);
		mSelectBtn.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if (mChosenFile != null) {
					getIntent().putExtra(CHOSEN_PATH, mChosenFile.getPath());
					setResult(RESULT_OK, getIntent());
					finish();
				}
			}
		});		
						
		mSuffix = getIntent().getStringArrayExtra(FILE_SUFFIX);
		mSupportDir = getIntent().getBooleanExtra(SUPPORT_DIR, false);	
		String szPath = getIntent().getStringExtra(ROOT_PATH);
		if (szPath == null) {
			szPath = ROOT;
		}
		if (mSupportDir) {
			File file = new File(szPath);
			mChosenFile = file;
			mSelectBtn.setEnabled(true);
		}
		loadPath(szPath);
	}

	private void loadPath(String path) {
		boolean useAutoSelection = path.length() < mCurrPath.length();
		Integer position = mLastPositions.get(mParentPath);
		realLoadPath(path);
		if (position != null && useAutoSelection) {
			getListView().setSelection(position);
		}
	}

	private void realLoadPath(final String path) {
		mCurrPath = path;		
		mPathList = new ArrayList<String>();
		mFileList = new ArrayList<HashMap<String, Object>>();

		File f = new File(mCurrPath);
		File[] files = f.listFiles();
		if (files == null) {
			mCurrPath = ROOT;
			f = new File(mCurrPath);
			files = f.listFiles();
		}
		mPathView.setText(getText(R.string.pathLocation) + ": " + mCurrPath);

		final List<String> item = new ArrayList<String>();
		if (!mCurrPath.equals(ROOT)) {
			item.add(ROOT);
			addFile(ROOT, R.drawable.folder_logo);
			mPathList.add(ROOT);

			item.add("../");
			addFile("../", R.drawable.folder_logo);
			mPathList.add(f.getParent());
			mParentPath = f.getParent();
		}

		TreeMap<String, String> dirsMap = new TreeMap<String, String>();
		TreeMap<String, String> dirsPathMap = new TreeMap<String, String>();
		TreeMap<String, String> filesMap = new TreeMap<String, String>();
		TreeMap<String, String> filesPathMap = new TreeMap<String, String>();
		for (File file : files) {
			if (file.isDirectory()) {
				String dirName = file.getName();
				dirsMap.put(dirName, dirName);
				dirsPathMap.put(dirName, file.getPath());
			} else {
				final String fileName = file.getName();
				if (mSuffix != null) {
					for (int i = 0; i < mSuffix.length; i++) {
						if (fileName.toLowerCase().endsWith(mSuffix[i].toLowerCase())) {
							filesMap.put(fileName, fileName);
							filesPathMap.put(fileName, file.getPath());
							break;
						}
					}
				} else {
					filesMap.put(fileName, fileName);
					filesPathMap.put(fileName, file.getPath());
				}
			}
		}
		item.addAll(dirsMap.tailMap("").values());
		item.addAll(filesMap.tailMap("").values());
		mPathList.addAll(dirsPathMap.tailMap("").values());
		mPathList.addAll(filesPathMap.tailMap("").values());

		SimpleAdapter fileList = new SimpleAdapter(this, mFileList, R.layout.localfilerow, 
				new String[] {ITEM_NAME, ITEM_LOGO }, 
				new int[] { R.id.fdrowtext, R.id.fdrowimage });
		
		for (String dir : dirsMap.tailMap("").values()) {
			addFile(dir, R.drawable.folder_logo);
		}

		for (String file : filesMap.tailMap("").values()) {
			addFile(file, R.drawable.file_logo);
		}

		fileList.notifyDataSetChanged();
		setListAdapter(fileList);
	}

	private void addFile(String name, int logo) {
		HashMap<String, Object> item = new HashMap<String, Object>();
		item.put(ITEM_NAME, name);
		item.put(ITEM_LOGO, logo);
		mFileList.add(item);
	}

	@Override
	protected void onListItemClick(ListView list, View view, int position, long id) {
		File file = new File(mPathList.get(position));
		mSelectBtn.setEnabled(false);
		
		if (list.getTag() != null){
		    ((View)list.getTag()).setBackgroundDrawable(null);
		}
		list.setTag(view);
		view.setBackgroundColor(Color.BLUE);

		if (file.isDirectory()) {
			mSelectBtn.setEnabled(false);
			if (file.canRead()) {
				mLastPositions.put(mCurrPath, position);
				loadPath(mPathList.get(position));
				if (mSupportDir) {
					mChosenFile = file;
					view.setSelected(true);
					mSelectBtn.setEnabled(true);
				}
			} else {
				AlertDialog.Builder bld = new AlertDialog.Builder(this);
				bld.setIcon(R.drawable.dialog_logo);
				bld.setTitle("[" + file.getName() + "] " + getText(R.string.canNotAccess));
				bld.setPositiveButton("OK", new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
							}
							}).show();
			}
		} else {			
			mChosenFile = file;
			view.setSelected(true);
			mSelectBtn.setEnabled(true);
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if ((keyCode == KeyEvent.KEYCODE_BACK)) {
			mSelectBtn.setEnabled(false);
			if (!mCurrPath.equals(ROOT)) {
				loadPath(mParentPath);
				return true;
			} else {
				return super.onKeyDown(keyCode, event);
			}			
		} else {
			return super.onKeyDown(keyCode, event);
		}
	}

}
