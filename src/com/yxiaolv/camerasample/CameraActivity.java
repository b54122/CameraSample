package com.yxiaolv.camerasample;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import com.yxiaolv.camerasample.R;

import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.media.MediaRecorder;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Toast;

public class CameraActivity extends Activity {
	
	static final String TAG =  "CAMERA ACTIVITY"; 

	//Camera object
	Camera mCamera1;
	Camera mCamera2;
	Camera mCamera3;
	Camera mCamera4;
	//Preview surface
	SurfaceView surfaceView0;
	SurfaceView surfaceView1;
	SurfaceView surfaceView2;
	SurfaceView surfaceView3;
	//Preview surface handle for callback
	SurfaceHolder surfaceHolder0;
	SurfaceHolder surfaceHolder1;
	SurfaceHolder surfaceHolder2;
	SurfaceHolder surfaceHolder3;
	//Camera button
	Button btnCapture;
	//Note if preview windows is on.
	boolean isRecording = false;
	
/*andler mThread0 = new Handler();
	Handler mThread1 = new Handler();
	Handler mThread2 = new Handler();
	Handler mThread3 = new Handler();
	Handler rThread0 = new Handler();
	Handler rThread1 = new Handler();
	Handler rThread2 = new Handler();
	Handler rThread3 = new Handler();
*/

	
	String PATH = Environment.getExternalStorageDirectory().getAbsolutePath();
	
	int mCurrentCamIndex = 0;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		surfaceView0 = (SurfaceView) findViewById(R.id.surfaceView0);
		surfaceHolder0 = surfaceView0.getHolder();
		surfaceView1 = (SurfaceView) findViewById(R.id.surfaceView1);
		surfaceHolder1 = surfaceView1.getHolder();
		surfaceView2 = (SurfaceView) findViewById(R.id.surfaceView2);
		surfaceHolder2 = surfaceView2.getHolder();
		surfaceView3 = (SurfaceView) findViewById(R.id.surfaceView3);
		surfaceHolder3 = surfaceView3.getHolder();
		
		LoadVideo(surfaceHolder0,0);
		LoadVideo(surfaceHolder1,1);
		LoadVideo(surfaceHolder2,2);
		LoadVideo(surfaceHolder3,3);
		btnCapture = (Button) findViewById(R.id.btn_capture);
		btnCapture.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View arg0) {
				
			}
		});
	}
	 private Handler handler = new Handler();
     private ExecutorService executorService = Executors.newFixedThreadPool(4);
     // 引入线程池来管理多线程
     private void LoadVideo(final SurfaceHolder surfaceHolder,final int camIdx) {
         executorService.submit(new Runnable() {
             public void run() {
                     try {
                     handler.post(new Runnable() {
                         public void run() {
                        	 
                        		switch(camIdx){
                        		case 0:
                        			surfaceHolder.addCallback(new SurfaceViewCallback1());
                        			break;
                        		case 1:
                        			surfaceHolder.addCallback(new SurfaceViewCallback2());
                        			break;
                        		case 2:
                        			surfaceHolder.addCallback(new SurfaceViewCallback3());
                        			break;
                        		case 3:
                        			surfaceHolder.addCallback(new SurfaceViewCallback4());
                        			break;
                        		} 		
                        		//surfaceHolder.addCallback(this);
                        	surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
                         }
                     });
                     } catch (Exception e) {
                             throw new RuntimeException(e);
                     }
             }
         });
     }

	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	ShutterCallback shutterCallback = new ShutterCallback() {
		@Override
		public void onShutter() {
		}
	};	
	
	PictureCallback rawPictureCallback = new PictureCallback() {
		@Override
		public void onPictureTaken(byte[] arg0, Camera arg1) {

		}
	};
	
	PictureCallback jpegPictureCallback = new PictureCallback() {
		@Override
		public void onPictureTaken(byte[] arg0, Camera arg1) {

			String fileName = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)
					.toString()
					+ File.separator
					+ "PicTest_" + System.currentTimeMillis() + ".jpg";
			File file = new File(fileName);
			if (!file.getParentFile().exists()) {
				file.getParentFile().mkdir();
			}
			
			try {
				BufferedOutputStream bos = new BufferedOutputStream(
						new FileOutputStream(file));
				bos.write(arg0);
				bos.flush();
				bos.close();		
				scanFileToPhotoAlbum(file.getAbsolutePath());
				Toast.makeText(CameraActivity.this, "[Test] Photo take and store in" + file.toString(),Toast.LENGTH_LONG).show();
			} catch (Exception e) {
				Toast.makeText(CameraActivity.this, "Picture Failed" + e.toString(),
						Toast.LENGTH_LONG).show();
			}
		};
	};
	
	public void scanFileToPhotoAlbum(String path) {

        MediaScannerConnection.scanFile(CameraActivity.this,
                new String[] { path }, null,
                new MediaScannerConnection.OnScanCompletedListener() {

                    public void onScanCompleted(String path, Uri uri) {
                        Log.i("TAG", "Finished scanning " + path);
                    }
                });
    }
	private final class SurfaceViewCallback1 implements android.view.SurfaceHolder.Callback,Camera.PreviewCallback {   
		
		public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) 
		{
	
			try {
				mCamera1.setPreviewDisplay(arg0);
				mCamera1.startPreview();
				mCamera1.setPreviewCallback(this);
				setCameraDisplayOrientation(CameraActivity.this, mCurrentCamIndex, mCamera1);
			} catch (Exception e) {}
		}
		public void surfaceCreated(SurfaceHolder holder) {
//				mCamera = Camera.open();
			    //change to front camera
			    mCamera1 = openFrontFacingCameraGingerbread(0);
				// get Camera parameters
				Camera.Parameters params = mCamera1.getParameters();

				List<String> focusModes = params.getSupportedFocusModes();
				if (focusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO)) {
				  // Autofocus mode is supported
				}
			}

			public void surfaceDestroyed(SurfaceHolder holder) {
				mCamera1.stopPreview();
				mCamera1.release();
				mCamera1 = null;
			}
			public void onPreviewFrame(byte[] data, Camera camera){
				FileOutputStream outStream = null;
				try {
				YuvImage yuvimage = new YuvImage(data,ImageFormat.NV21,camera.getParameters().getPreviewSize 
				().width,camera.getParameters().getPreviewSize().height,null);
				ByteArrayOutputStream baos = new ByteArrayOutputStream();
				yuvimage.compressToJpeg(new Rect(0,0,camera.getParameters().getPreviewSize().width,camera.getParameters 
				().getPreviewSize().height), 80, baos);

				outStream = new FileOutputStream(String.format("/sdcard/video0.jpg"));
//				outStream.write(baos.toByteArray());
//				outStream.close();
				
				
				Log.d(TAG, "onPreviewFrame - wrote bytes: " + data.length);
				} catch (FileNotFoundException e) {
				e.printStackTrace();
				} catch (IOException e) {
				e.printStackTrace();
				} finally {
				}
//				Preview.this.invalidate();		
//				Log.v(TAG, mat+"");
			}
	}
	
	private final class SurfaceViewCallback2 implements android.view.SurfaceHolder.Callback,Camera.PreviewCallback {   
		public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) 
		{
	
			try {
				mCamera2.setPreviewDisplay(arg0);
				mCamera2.startPreview();
				mCamera2.setPreviewCallback(this);
				setCameraDisplayOrientation(CameraActivity.this, mCurrentCamIndex, mCamera2);
			} catch (Exception e) {}
		}
		public void surfaceCreated(SurfaceHolder holder) {
//				mCamera = Camera.open();
			    //change to front camera
			    mCamera2 = openFrontFacingCameraGingerbread(1);
				// get Camera parameters
				Camera.Parameters params = mCamera2.getParameters();

				List<String> focusModes = params.getSupportedFocusModes();
				if (focusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO)) {
				  // Autofocus mode is supported
				}
		}

		public void surfaceDestroyed(SurfaceHolder holder) {
			mCamera2.stopPreview();
			mCamera2.release();
			mCamera2 = null;
		}
		public void onPreviewFrame(byte[] data, Camera camera){
/*			Mat mat = new Mat(camera.getParameters().getPreviewSize 
					().width, camera.getParameters().getPreviewSize().height, CvType.CV_8UC3);
			mat.put(0, 0, data);*/
		}
	}
	
	private final class SurfaceViewCallback3 implements android.view.SurfaceHolder.Callback,Camera.PreviewCallback {   
		public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) 
		{
			try {
				mCamera3.setPreviewDisplay(arg0);
				mCamera3.startPreview();
				mCamera3.setPreviewCallback(this);
				setCameraDisplayOrientation(CameraActivity.this, mCurrentCamIndex, mCamera3);
			} catch (Exception e) {}
		}
		public void surfaceCreated(SurfaceHolder holder) {
//				mCamera = Camera.open();
			    //change to front camera
			    mCamera3 = openFrontFacingCameraGingerbread(2);
				// get Camera parameters
				Camera.Parameters params = mCamera3.getParameters();

				List<String> focusModes = params.getSupportedFocusModes();
				if (focusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO)) {
				  // Autofocus mode is supported
				}
			}

			public void surfaceDestroyed(SurfaceHolder holder) {
				mCamera3.stopPreview();
				mCamera3.release();
				mCamera3 = null;
			}
			public void onPreviewFrame(byte[] data, Camera camera){
/*				Mat mat = new Mat(camera.getParameters().getPreviewSize 
						().width, camera.getParameters().getPreviewSize().height, CvType.CV_8UC3);
				mat.put(0, 0, data);*/
			}
	}
	
	private final class SurfaceViewCallback4 implements android.view.SurfaceHolder.Callback,Camera.PreviewCallback {   
		public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) 
		{
			try {
				mCamera4.setPreviewDisplay(arg0);
				mCamera4.startPreview();
				mCamera4.setPreviewCallback(this);
				setCameraDisplayOrientation(CameraActivity.this, mCurrentCamIndex, mCamera4);
			} catch (Exception e) {}
		}
		public void surfaceCreated(SurfaceHolder holder) {
//				mCamera = Camera.open();
			    //change to front camera
			    mCamera4 = openFrontFacingCameraGingerbread(3);
				// get Camera parameters
				Camera.Parameters params = mCamera4.getParameters();

				List<String> focusModes = params.getSupportedFocusModes();
				if (focusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO)) {
				  // Autofocus mode is supported
				}
			}

			public void surfaceDestroyed(SurfaceHolder holder) {
				mCamera4.stopPreview();
				mCamera4.release();
				mCamera4 = null;
			}
			public void onPreviewFrame(byte[] data, Camera camera){
/*				Mat mat = new Mat(camera.getParameters().getPreviewSize 
						().width, camera.getParameters().getPreviewSize().height, CvType.CV_8UC3);
				mat.put(0, 0, data);*/
			}
			
	}
	
	private Camera openFrontFacingCameraGingerbread(int camIdx) {
	    int cameraCount = 4;
	    Camera cam = null;
//	    Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
//	    cameraCount = Camera.getNumberOfCameras();
	     if(camIdx < cameraCount) {
//	        Camera.getCameraInfo(camIdx, cameraInfo);
	            try {
	                cam = Camera.open(camIdx);
	                mCurrentCamIndex = camIdx;
	            } catch (RuntimeException e) {
	                Log.e(TAG, "Camera failed to open: " + e.getLocalizedMessage());
	            }
	    }

	    return cam;
	}
	
	//根据横竖屏自动调节preview方向，Starting from API level 14, this method can be called when preview is active.
	private static void setCameraDisplayOrientation(Activity activity,int cameraId, Camera camera) 
	{    
		   Camera.CameraInfo info = new Camera.CameraInfo(); 
	       Camera.getCameraInfo(cameraId, info);      
	       int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
	       
	       //degrees  the angle that the picture will be rotated clockwise. Valid values are 0, 90, 180, and 270. 
	       //The starting position is 0 (landscape). 
	       int degrees = 0;
	       switch (rotation) 
	       {   
	           case Surface.ROTATION_0: degrees = 0; break;         
	           case Surface.ROTATION_90: degrees = 90; break;    
	           case Surface.ROTATION_180: degrees = 180; break; 
	           case Surface.ROTATION_270: degrees = 270; break;  
	        }      
	       int result;  
	       if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT)
	       {        
	           result = (info.orientation + degrees) % 360;     
	           result = (360 - result) % 360;  // compensate the mirror   
	       } 
	       else 
	       {  
	       // back-facing       
	          result = (info.orientation - degrees + 360) % 360;   
	       }     
	       camera.setDisplayOrientation(result);  
	} 
	

}
