"""
Script Name: rp2040_reciever.py
Author: Lydia Unterreiner
Date: March 12, 2024

Description: This script demonstrates Bluetooth communication with an RP2040 device.
Data read from the RP2040 includes accelerometer, gyroscope, temperature and microphone. 
This data will then be saved to their respective CSV files
"""

import time 
import datetime
import csv
import asyncio
from bleak import BleakScanner
from bleak import BleakClient


accelerometer_data = []
gryoscope_data = []
temperate_data = []
microphone_data = []

address = ""
MODEL_NBR_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"


def write_data_to_csv():
    # write accel csv
    fields = ['datetime', 'Ax', 'Ay', 'Az']
    filename = "accelerometer_records.csv"

    with open(filename, 'w') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fields)
        writer.writeheader()
        writer.writerows(accelerometer_data)
            
    time.sleep(1) 

    # write gyro csv
    fields = ['datetime', 'Gx', 'Gy', 'Gz']
    filename = "gyroscope_records.csv"

    with open(filename, 'w') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fields)
        writer.writeheader()
        writer.writerows(gryoscope_data)

    time.sleep(1)

    # write temp csv
    fields = ['datetime', 'celsius']
    filename = "temperature_records.csv"

    with open(filename, 'w') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fields)
        writer.writeheader()
        writer.writerows(temperate_data)
            
    time.sleep(1)

    # write mic csv
    fields = ['datetime', 'Hz']
    filename = "microphone_records.csv"

    with open(filename, 'w') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fields)
        writer.writeheader()
        writer.writerows(microphone_data)

async def notification_handler(sender: int, data: bytearray):
    #print(f"Received data: {data.decode()}")
    
    ct = datetime.datetime.now()
    data = data.decode()
    string = str(data)

    if "Accel" in string:
        string_split = string.split(" ")

        accel_data = {}
        accel_data["datetime"] = ct
        accel_data["Ax"] = string_split[1]
        accel_data["Ay"] = string_split[2]
        accel_data["Az"] = string_split[3]
        
        print(accel_data) 
        accelerometer_data.append(accel_data)

    
    if "Gyro" in string: 
        string_split = string.split(" ")

        gyro_data = {}
        gyro_data["datetime"] = ct
        gyro_data["Gx"] = string_split[1]
        gyro_data["Gy"] = string_split[2]
        gyro_data["Gz"] = string_split[3]

        print(gyro_data) 
        gryoscope_data.append(gyro_data)

    
    if "Temp" in string:

        string_split = string.split(" ")

        temp_data = {}
        temp_data["datetime"] = ct
        temp_data["celsius"] = string_split[1]

        print(temp_data) 
        temperate_data.append(temp_data)

    
    if "Mic" in string:

        string_split = string.split(" ")
        
        mic_data = {}
        mic_data["datetime"] = ct
        mic_data["Hz"] = string_split[2]

        print(mic_data) 
        microphone_data.append(mic_data)

async def connect_to_device(address):
    client = BleakClient(address)
    await client.connect()
    print("Client connected")
    return client

async def disconnect_from_device(client):
    await client.stop_notify(MODEL_NBR_UUID)
    await client.disconnect()
    print("Client disconnected")



async def main():
    
    try:
        while True:
            devices = await BleakScanner.discover()
            for device in devices:
                if device.name != None and "RP2040" in device.name:
                    address = device.address

            client = await connect_to_device(address)
            
            # Subscribe to notifications
            await client.start_notify(MODEL_NBR_UUID, notification_handler)

    except Exception as e:
        print(e)

    except KeyboardInterrupt:
        print("Script interrupted. Disconnecting from the device...")

    finally:
        if 'client' in locals():
            await disconnect_from_device(client)
        
        write_data_to_csv()          

asyncio.run(main())