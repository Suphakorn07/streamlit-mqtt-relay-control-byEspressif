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

# ปรับธีมและการตั้งค่าเพจ
st.set_page_config(page_title="Relay Control Dashboard", page_icon="🔌", layout="centered")

# Custom CSS with background image, animated buttons, and dark text
st.markdown("""
<style>
    /* พื้นหลังแบบภาพ */
    .stApp {
        background-size: cover;
        background-position: center;
        background-attachment: fixed;
        background : #F9E2AF;
    }

    /* ปรับแต่งปุ่ม */
    .stButton>button {
        color: #FCFAEE;
        background-color: #151515;
        border-radius: 50px;
        display: block;
        margin: 0 auto;
        width: 200px;
        height: 70px;
        font-size: 24px;
        font-weight: bold;
        border: 2px solid #FCFAEE;
        transition: background-color 0.3s, color 0.3s;
    }

    /* ปุ่ม hover */
    .stButton>button:hover {
        background-color: #FCFAEE;
        color: #151515;
        border: 2px solid #151515;
        transform: scale(1.1);
    }

    /* การจัดการ title ให้ตรงกลาง */
    .centered-title {
        text-align: center;
        color: #151515;
        font-size: 3rem;
        font-weight: bold;
        margin-bottom: 3rem;
    }

    /* การจัดการปุ่มให้อยู่ตรงกลาง */
    .button-container {
        display: flex;
        justify-content: center;
        gap: 50px;
        margin-top: 50px;
    }
</style>
""", unsafe_allow_html=True)

# ส่วนของ Streamlit app
st.markdown("<h1 class='centered-title'>Relay Control Dashboard</h1>", unsafe_allow_html=True)

# ปุ่มควบคุมรีเลย์
st.markdown("<div class='button-container'>", unsafe_allow_html=True)
col1, col2 = st.columns(2)
with col1:
    if st.button('ON', key='on_button'):
        control_relay("on")
with col2:
    if st.button('OFF', key='off_button'):
        control_relay("off")
st.markdown("</div>", unsafe_allow_html=True)
