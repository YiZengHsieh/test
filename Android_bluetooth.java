package com.example.drawingcars;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ArrayBlockingQueue;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Handler;
import android.util.Log;

public class BluetoothCommunication{
	
	public final ArrayBlockingQueue<Integer> buffer = new ArrayBlockingQueue(1);
	private static final String DEVICE_ADDRESS =  "20:13:01:24:06:29";
	public String receiveData;
	// The component that bluetooth would need.
	BluetoothAdapter mBluetoothAdapter;
	BluetoothSocket mmSocket;
	BluetoothDevice mmDevice;
	OutputStream mmOutputStream;
	InputStream mmInputStream;
	Thread workerThread;
	byte[] readBuffer;
	int readBufferPosition;
	int counter;
	volatile boolean stopWorker;
	int flag = 0;
	
	public void findBT(){
    	mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    	if(mBluetoothAdapter == null){
    		Log.w("mBluetoothAdapter", "no adapter");
    	}
    	
    	/*if(!mBluetoothAdapter.isEnabled()){
    		Intent enableBluetooth = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
    		startActivityForResult(enableBluetooth, 0);
    	}*/
    	
    	Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        if(pairedDevices.size() > 0)
        {
            for(BluetoothDevice device : pairedDevices)
            {
                if(device.getName().equals("bear")) 
                {
                    mmDevice = device;
                    break;
                }
            }
        }
        
        Log.w("bluetooth device", "Bluetooth Device Found");
    }
	
	public void openBT() throws IOException{
    	UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805f9b34fb"); //Standard SerialPortService ID
        mmSocket = mmDevice.createRfcommSocketToServiceRecord(uuid);        
        mmSocket.connect();
        mmOutputStream = mmSocket.getOutputStream();
        mmInputStream = mmSocket.getInputStream();
        //beginListenForData();
        Log.w("Bluetooth", "Bluetooth Opened");
        
    }
	
	void beginListenForData()
    {
    	Log.w("beginListenForData", "beginListenForData");
        final Handler handler = new Handler(); 
        final byte delimiter = 10; //This is the ASCII code for a newline character
        
        stopWorker = false;
        readBufferPosition = 0;
        readBuffer = new byte[1024];
        workerThread = new Thread(new Runnable()
        {
        	
            public void run()
            {                
            	Log.w("workerThread", "enterThread");
               while(!Thread.currentThread().isInterrupted() && !stopWorker)
               {	
                    try 
                    {
                    	
                    	int bytesAvailable = mmInputStream.available();                        
                        if(bytesAvailable > 0)
                        {
                        	Log.w("bytesAvailable", "bytesAvailable");
                            byte[] packetBytes = new byte[bytesAvailable];
                            mmInputStream.read(packetBytes);
                            for(int i = 0 ; i < bytesAvailable ; i++)
                            {
                                byte b = packetBytes[i];
                                //readBuffer[readBufferPosition++] = b;
                                if(b == delimiter)
                                {
                                    byte[] encodedBytes = new byte[readBufferPosition];
                                    System.arraycopy(readBuffer, 0, encodedBytes, 0, encodedBytes.length);
                                    final String data = new String(encodedBytes, "US-ASCII");
                                    readBufferPosition = 0;
                                    Log.w("Bluetooth read", "readData");
                                    
                                    
                                    handler.post(new Runnable()
                                    {
                                        public void run()
                                        {
                                        		Log.w("Handler", "post");
                                        		receiveData = data;
                                                
                                            	/*try {
    												//buffer.put(Integer.valueOf(data));
    											} catch (NumberFormatException e) {
    												// TODO Auto-generated catch block
    												e.printStackTrace();
    											} */
                                        }
                                    });
                                }
                                else
                                {
                                    readBuffer[readBufferPosition++] = b;
                                }
                            }
                        }
                    } 
                    catch (IOException ex) 
                    {
                        stopWorker = true;
                    }
               }
            }
        });
        
        workerThread.start();
    }
	
	int setReceiveData(String data){
		receiveData = data;
		Log.w("test1", data);
		return Integer.valueOf(receiveData);
		
	}

	public String getReceiveData(){
		return receiveData;
	}
	
	
	void closeBT() throws IOException
    {
        stopWorker = true;
        mmOutputStream.close();
        mmInputStream.close();
        mmSocket.close();
        //myLabel.setText("Bluetooth Closed");
        Log.w("bluetooth closed", "Bluetooth Closed");
    }
	
	void sendData(int message) throws IOException
    {
		Log.w("data sent", String.valueOf(message));
		mmOutputStream.write((byte)message);
        //myLabel.setText("Data Sent");
        Log.w("data sent", "Data Sent");
        
    }
	
	void sendData(String message) throws IOException
    {
		Log.w("data sent", String.valueOf(message));
		mmOutputStream.write(message.getBytes());
        //myLabel.setText("Data Sent");
        Log.w("data sent", "Data Sent");
        
    }
}
