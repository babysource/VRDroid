/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid;

import android.content.Intent;
import android.graphics.Bitmap;
import android.media.ThumbnailUtils;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.app.FragmentActivity;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * VR资源列表
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidLister extends FragmentActivity {

    private final static String VR_FOLDER = "360Videos";

    private List<Bitmap> mVrVideoIcon = new ArrayList<>();

    private long mBackPressedTime;

    private ListView mVrVideoList;

    private VRDroidAdapter mVrAdapter;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.droid_lister);
        this.mVrVideoList = (ListView) this.findViewById(R.id.vr_video_list);
        if (this.mVrVideoList != null) {
            this.mVrVideoList.setAdapter(
                    this.mVrAdapter = new VRDroidAdapter()
            );
        }
        if (this.mVrAdapter != null) {
            final File folder = new File(Environment.getExternalStorageDirectory() + File.separator + VR_FOLDER);
            if (!folder.exists()) {
                folder.mkdirs();
            }
            final File[] files = folder.listFiles();
            if (files.length > 0) {
                final String[] paths = new String[files.length];
                for (int i = 0, l = files.length; i < l; i++) {
                    paths[i] = files[i].getAbsolutePath();
                }
                new VRIconTask().execute(paths);
                this.mVrAdapter.setFiles(files);
            }
        }
    }

    @Override
    public void onBackPressed() {
        final long mCurrentTime = System.currentTimeMillis();
        if (mCurrentTime - this.mBackPressedTime > 1000) {
            Toast.makeText(this, R.string.app_logout, Toast.LENGTH_SHORT).show();
            this.mBackPressedTime = mCurrentTime;
            return;
        }
        super.onBackPressed();
        System.exit(0);
    }

    private final class VRDroidAdapter extends BaseAdapter {

        private File[] files;

        public void setFiles(final File[] files) {
            this.files = files;
        }

        @Override
        public int getCount() {
            return files != null ? files.length : 0;
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            VRViewHolder holder = null;
            if (convertView != null) {
                holder = (VRViewHolder) convertView.getTag();
            } else {
                convertView = LayoutInflater.from(VRDroidLister.this).inflate(R.layout.droid_item, null);
                if (convertView != null) {
                    convertView.setTag(
                            holder = new VRViewHolder(convertView)
                    );
                }
            }
            if (holder != null) {
                final File file = this.files[position];
                if (file != null) {
                    final String path = file.getAbsolutePath();
                    if (!TextUtils.isEmpty(path)) {
                        final String name = this.getFileName(file);
                        if (!TextUtils.isEmpty(name)) {
                            holder.name.setText(name);
                        }
                        if (mVrVideoIcon.size() > position) {
                            holder.icon.setImageBitmap(mVrVideoIcon.get(position));
                        }
                        convertView.setOnClickListener(new View.OnClickListener() {
                            @Override
                            public void onClick(final View v) {
                                final Intent intent = new Intent();
                                intent.putExtra(VRDroidPlayer.KEY_NAME, name);
                                intent.putExtra(VRDroidPlayer.KEY_PATH, path);
                                intent.setClass(VRDroidLister.this, VRDroidPlayer.class);
                                startActivity(intent);
                            }
                        });
                    }
                }
            }
            return convertView;
        }

        private String getFileName(final File file) {
            final String name = file.getName();
            if (!TextUtils.isEmpty(name)) {
                final int ext = name.lastIndexOf(".");
                if (ext > 0) {
                    return name.substring(0, ext);
                }
            }
            return name;
        }

    }

    private final class VRViewHolder {
        final TextView name;
        final ImageView icon;

        public VRViewHolder(final View view) {
            this.name = (TextView) view.findViewById(R.id.vr_video_name);
            this.icon = (ImageView) view.findViewById(R.id.vr_video_icon);
        }
    }

    private final class VRIconTask extends AsyncTask<String, Void, Void> {

        @Override
        protected Void doInBackground(final String... paths) {
            for (final String path : paths) {
                mVrVideoIcon.add(ThumbnailUtils.createVideoThumbnail(path, MediaStore.Video.Thumbnails.MICRO_KIND));
            }
            return null;
        }

        @Override
        protected void onPostExecute(final Void aVoid) {
            if (mVrAdapter != null) {
                mVrAdapter.notifyDataSetChanged();
            }
        }
    }

}
