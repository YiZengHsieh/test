package com.example.positionbeacon;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;
import android.widget.AbsoluteLayout;
import android.widget.ImageView;
import android.widget.Toast;
import com.aprilbrother.aprilbrothersdk.Beacon;
import com.aprilbrother.aprilbrothersdk.BeaconManager;
import com.aprilbrother.aprilbrothersdk.BeaconManager.MonitoringListener;
import com.aprilbrother.aprilbrothersdk.BeaconManager.RangingListener;
import com.aprilbrother.aprilbrothersdk.Region;

/**
 *  搜尋並顯示Beacon
 */
public class MainActivity extends Activity {
	private ImageView user; 
	private static final int REQUEST_ENABLE_BT = 1234;
	private static final String TAG = "BeaconList";
	private static final Region ALL_BEACONS_REGION = new Region("apr", null,
			null, null);
	private BeaconManager beaconManager;
	private ArrayList<Beacon> myBeacons;
	private DecimalFormat df;
	int minnor1,minnor2,minnor3;
	double d1,d2,d3;
	double xa=0.0,xb=5.0,xc=10.0,ya=5.0,yb=0.0,yc=5.0;
	double s,t,x,y;
	int r1,r2,r3;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		user = (ImageView)findViewById(R.id.user);
		init();
		
	}

	/**
	 * 初始化操作
	 */
	private void init() {
		df=new DecimalFormat("#.##");
		myBeacons = new ArrayList<Beacon>();
		beaconManager = new BeaconManager(this);
		beaconManager.setMonitoringExpirationMill(10L);
		beaconManager.setRangingExpirationMill(10L);
		beaconManager.setRangingListener(new RangingListener() {

			@Override
			public void onBeaconsDiscovered(Region region,
					final List<Beacon> beacons) {
				myBeacons.addAll(beacons);
				runOnUiThread(new Runnable() {
					@Override
					public void run() {
						getActionBar().setSubtitle("Found beacons: "+beacons.size()+","+x+","+(250-y));
						for(int i=0;i<3;i++){
							if(beacons.get(i).getMinor()==0){
								d1=Double.parseDouble(df.format(beacons.get(i).getDistance()));
								r1=(int)((d1*100)/25);
							}else if(beacons.get(i).getMinor()==1){
								d2=Double.parseDouble(df.format(beacons.get(i).getDistance()));
								r2=(int)((d2*100)/25);
							}else if(beacons.get(i).getMinor()==2){
								d3=Double.parseDouble(df.format(beacons.get(i).getDistance()));
								r3=(int)((d3*100)/25);
							}
						}
						s = (Math.pow(xc, 2.) - Math.pow(xb, 2.) + Math.pow(yc, 2.) - Math.pow(yb, 2.) + Math.pow(r2, 2.) - Math.pow(r3, 2.)) / 2.0;
						t = (Math.pow(xa, 2.) - Math.pow(xb, 2.) + Math.pow(ya, 2.) - Math.pow(yb, 2.) + Math.pow(r2, 2.) - Math.pow(r1, 2.)) / 2.0;
						y = ((t * (xb - xc)) - (s * (xb - xa))) / (((ya - yb) * (xb - xc)) - ((yc - yb) * (xb - xa)));
						x = ((y * (ya - yb)) - t) / (xb - xa);
						user.setLayoutParams(
						        new AbsoluteLayout.LayoutParams(48, 48, (int)x, (250-(int)y))
							      );
					}
				});
			}
		});
		
		beaconManager.setMonitoringListener(new MonitoringListener() {
			
			@Override
			public void onExitedRegion(Region arg0) {
				Toast.makeText(MainActivity.this, "Beacon訊號遺失", 0).show();
				
			}
			
			@Override
			public void onEnteredRegion(Region arg0, List<Beacon> arg1) {
				Toast.makeText(MainActivity.this, "偵測到Beacon訊號", 0).show();
			}
		});
	}
	
	/**
	 * 開始搜尋Beacon
	 */
	private void connectToService() {
		getActionBar().setSubtitle("Scanning...");
		beaconManager.connect(new BeaconManager.ServiceReadyCallback() {
			@Override
			public void onServiceReady() {
				try {
					beaconManager.startRanging(ALL_BEACONS_REGION);
					beaconManager.startMonitoring(ALL_BEACONS_REGION);
				} catch (RemoteException e) {
					
				}
			}
		});
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == REQUEST_ENABLE_BT) {
			if (resultCode == Activity.RESULT_OK) {
				connectToService();
			} else {
				Toast.makeText(this, "Bluetooth not enabled", Toast.LENGTH_LONG)
						.show();
				getActionBar().setSubtitle("Bluetooth not enabled");
			}
		}
		super.onActivityResult(requestCode, resultCode, data);
	}

	@Override
	protected void onStart() {
		super.onStart();

		if (!beaconManager.hasBluetooth()) {
			Toast.makeText(this, "Device does not have Bluetooth Low Energy",
					Toast.LENGTH_LONG).show();
			return;
		}

		if (!beaconManager.isBluetoothEnabled()) {
			Intent enableBtIntent = new Intent(
					BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
		} else {
			connectToService();
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	protected void onStop() {
		try {
			myBeacons.clear();
			beaconManager.stopRanging(ALL_BEACONS_REGION);
		} catch (RemoteException e) {
			Log.d(TAG, "Error while stopping ranging", e);
		}
		super.onStop();
	}
}
