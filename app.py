import streamlit as st
import paho.mqtt.client as mqtt

# MQTT broker configuration
mqtt_broker = "test.mosquitto.org"
topic_publish = "/topic/wpan"

# Create MQTT client
client = mqtt.Client()
client.connect(mqtt_broker, 1883, 60)

# Streamlit app configuration
st.title("Relay Control Dashboard")

# Control relay buttons
if st.button('Turn ON Relay'):
    client.publish(topic_publish, "on")
    st.success("Relay turned ON")

if st.button('Turn OFF Relay'):
    client.publish(topic_publish, "off")
    st.success("Relay turned OFF")
