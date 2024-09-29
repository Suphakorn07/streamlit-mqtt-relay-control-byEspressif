import streamlit as st
import paho.mqtt.client as mqtt

# MQTT broker configuration
mqtt_broker = "test.mosquitto.org"
mqtt_port = 8080  # ใช้พอร์ต 8080 สำหรับ WebSocket
mqtt_topic = "/topic/wpan"

# สร้าง MQTT client และตั้งค่า WebSocket
client = mqtt.Client(transport="websockets")
client.connect(mqtt_broker, mqtt_port, 60)




# ฟังก์ชันสำหรับส่งคำสั่งควบคุมรีเลย์
def control_relay(action):
    if action == "on":
        client.publish(mqtt_topic, "on")
        st.success("Relay turned ON")
    elif action == "off":
        client.publish(mqtt_topic, "off")
        st.success("Relay turned OFF")
    else:
        st.error("Invalid action")

# ส่วนของ Streamlit app
st.title("Relay Control Dashboard")

# ปุ่มควบคุมรีเลย์
if st.button('Turn ON Relay'):
    control_relay("on")

if st.button('Turn OFF Relay'):
    control_relay("off")

# ฟังก์ชัน callback เมื่อได้รับข้อความ MQTT

