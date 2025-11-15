package com.hip;

import com.alibaba.nls.client.protocol.NlsClient;
import com.alibaba.nls.client.protocol.commonrequest.CommonRequest;
import com.alibaba.nls.client.protocol.commonrequest.CommonRequestListener;
import com.alibaba.nls.client.protocol.commonrequest.CommonRequestResponse;

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import javax.sound.sampled.*;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class AudioEventDetectionDemo {

    public static final String TOKEN = "XXX";
    public static final String APPKEY = "XXX";

    private static final String NAMESPACE = "AudioEventDetection";
    private static final String URL = "wss://nls-gateway-cn-shanghai.aliyuncs.com/ws/v1";
    private static final int SAMPLE_RATE = 16000;
    private static final int CHUNK_DURATION = 200;

    // MQTT 配置
    private static final String MQTT_BROKER = "tcp://82.157.30.217:1883";
    private static final String MQTT_TOPIC = "device/vocality";
    private static final String MQTT_USER = "XXX";
    private static final String MQTT_PASS = "XXX";
    private static final String CLIENT_ID = "AudioEventDetector_" + UUID.randomUUID().toString();
    private static MqttClient mqttClient = null;

    private static final Map<String, String> EVENT_CN = new HashMap<>();
    private static final Map<String, Long> LAST_SPEAK_TIMES = new HashMap<>();
    private static final long DEBOUNCE_INTERVAL_MS = 5000;

    private static final ScheduledExecutorService scheduler = Executors.newScheduledThreadPool(1);

    static {
        EVENT_CN.put("Singing", "唱歌");
        EVENT_CN.put("Laughter", "笑声");
        EVENT_CN.put("Knock", "敲门声");
        EVENT_CN.put("Keyboard", "键盘声");
        EVENT_CN.put("Cry", "哭泣声");
        EVENT_CN.put("Explosion", "爆炸声");
        EVENT_CN.put("Water", "水声");
        EVENT_CN.put("SirenAlarm", "警笛声");
    }

    public static void main(String[] args) throws Exception {
        new Thread(() -> HttpSmsServer.start(8001)).start();
        initMqtt(); // 初始化 MQTT

        NlsClient client = new NlsClient(URL, TOKEN);

        AudioFormat format = new AudioFormat(SAMPLE_RATE, 16, 1, true, false);
        DataLine.Info info = new DataLine.Info(TargetDataLine.class, format);
        if (!AudioSystem.isLineSupported(info)) {
            System.err.println("麦克风不支持该格式: " + format);
            shutdownMqtt();
            return;
        }

        try (TargetDataLine line = (TargetDataLine) AudioSystem.getLine(info)) {
            line.open(format);
            line.start();

            CommonRequestListener listener = getListener();
            CommonRequest request = new CommonRequest(client, listener, NAMESPACE);
            request.setAppKey(APPKEY);
            request.addCustomedParam("format", "pcm");
            request.addCustomedParam("sample_rate", SAMPLE_RATE);
            request.start();

            int chunkSize = SAMPLE_RATE * 2 / 1000 * CHUNK_DURATION;
            byte[] buffer = new byte[chunkSize];

            System.out.println("开始识别: 实时音频事件检测");
            System.out.println("麦克风已开启，开始实时采集...（按 Ctrl+C 停止）");

            while (true) {
                int bytesRead = line.read(buffer, 0, buffer.length);
                if (bytesRead <= 0) continue;
                request.send(Arrays.copyOf(buffer, bytesRead));
            }
        } catch (Exception e) {
            System.err.println("录音或识别异常: " + e.getMessage());
            e.printStackTrace();
        } finally {
            client.shutdown();
            shutdownMqtt();
            scheduler.shutdown(); // 关闭调度器
        }
    }

    private static void initMqtt() {
        try {
            mqttClient = new MqttClient(MQTT_BROKER, CLIENT_ID);
            MqttConnectOptions options = new MqttConnectOptions();
            options.setUserName(MQTT_USER);
            options.setPassword(MQTT_PASS.toCharArray());
            options.setAutomaticReconnect(true);
            options.setCleanSession(true);
            mqttClient.connect(options);
            System.out.println("MQTT 连接成功: tcp://82.157.30.217:1883");
        } catch (MqttException e) {
            System.err.println("MQTT 连接失败: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private static void shutdownMqtt() {
        if (mqttClient != null && mqttClient.isConnected()) {
            try {
                mqttClient.disconnect();
                mqttClient.close();
                System.out.println("MQTT 已断开");
            } catch (MqttException e) {
                e.printStackTrace();
            }
        }
    }

    //发送控制命令
    private static void sendControlCommand(String cmd, String val) {
        if (mqttClient == null || !mqttClient.isConnected()) {
            System.err.println("MQTT 未连接，无法发送控制命令");
            return;
        }
        try {
            String payload = String.format("{\"cmd\":\"%s\",\"val\":\"%s\"}", cmd, val);
            MqttMessage message = new MqttMessage(payload.getBytes("UTF-8"));
            message.setQos(1);
            mqttClient.publish("device/control", message);
            System.out.println("控制命令已发送: " + payload);
        } catch (Exception e) {
            System.err.println("控制命令发送失败: " + e.getMessage());
        }
    }

    //触发告警动作（开5秒后关）
    private static void triggerAlarmActions() {
        // 开启
        sendControlCommand("buzzer", "ON");
        sendControlCommand("vibration", "ON");
        sendControlCommand("light", "red");

        SmsUtil.sendVerificationCode();
        // 5秒后关闭
        scheduler.schedule(() -> {
            sendControlCommand("buzzer", "OFF");
            sendControlCommand("vibration", "OFF");
            sendControlCommand("light", "OFF");
        }, 5, TimeUnit.SECONDS);
    }

    private static CommonRequestListener getListener() {
        return new CommonRequestListener() {
            @Override
            public void onStarted(CommonRequestResponse response) {
                System.out.println("开始识别任务: " + response.getTaskId());
            }

            @Override
            public void onEvent(CommonRequestResponse response) {
                Object payloadObj = response.payload;
                if (!(payloadObj instanceof Map)) return;

                @SuppressWarnings("unchecked")
                Map<String, Object> payload = (Map<String, Object>) payloadObj;
                Object resultObj = payload.get("result");
                if (!(resultObj instanceof List)) return;

                @SuppressWarnings("unchecked")
                List<Map<String, Object>> events = (List<Map<String, Object>>) resultObj;
                long now = System.currentTimeMillis();

                for (Map<String, Object> event : events) {
                    String type = getString(event, "event_type", null);
                    if (type == null || !EVENT_CN.containsKey(type)) continue;

                    double conf = getDouble(event, "confidence", 0.0);
                    long start = getLong(event, "start_time", 0L);
                    String cnType = EVENT_CN.get(type);
                    String msg = String.format("检测到 %s",cnType);

                    // 防抖
                    Long lastTime = LAST_SPEAK_TIMES.get(type);
                    if (lastTime == null || (now - lastTime) >= DEBOUNCE_INTERVAL_MS) {
                        LAST_SPEAK_TIMES.put(type, now);

                        System.out.printf("[%dms] %s (置信度: %.0f%%)\n", start, cnType, conf * 100);
                        speakChinese(msg);
                        sendMqttMessage(msg);

                        triggerAlarmActions();

                    } else {
                        System.out.printf("防抖过滤: %s\n", cnType);
                    }
                }
            }

            @Override
            public void onStopped(CommonRequestResponse response) {
                System.out.println("识别已停止: " + response.getTaskId());
            }

            @Override
            public void onFailed(CommonRequestResponse response) {
            }
        };
    }

    private static void sendMqttMessage(String text) {
        if (mqttClient == null || !mqttClient.isConnected()) {
            System.err.println(" MQTT 未连接，无法发送消息");
            return;
        }

        String jsonPayload = String.format("{\"text\":\"%s\"}", text.replace("\"", "\\\""));
        try {
            MqttMessage message = new MqttMessage(jsonPayload.getBytes("UTF-8"));
            message.setQos(1);
            mqttClient.publish(MQTT_TOPIC, message);
            System.out.println("MQTT 已发送到 " + MQTT_TOPIC + ": " + jsonPayload);

            //同时发送到 warning 主题（根据你之前的要求）
            mqttClient.publish("warning", message);
            System.out.println("MQTT 已发送到 warning: " + jsonPayload);
        } catch (Exception e) {
            System.err.println("MQTT 发送失败: " + e.getMessage());
        }
    }

    // 工具方法
    private static String getString(Map<String, Object> map, String key, String def) {
        Object obj = map.get(key);
        return obj != null ? obj.toString() : def;
    }

    private static double getDouble(Map<String, Object> map, String key, double def) {
        Object obj = map.get(key);
        if (obj instanceof Number) return ((Number) obj).doubleValue();
        return def;
    }

    private static long getLong(Map<String, Object> map, String key, long def) {
        Object obj = map.get(key);
        if (obj instanceof Number) return ((Number) obj).longValue();
        return def;
    }

    private static void speakChinese(String text) {
        try {
            String safeText = text.replace("'", "\\'").replace("\"", "\\\"");
            String cmd = String.format("espeak -v zh -s 150 '%s' --stdout | aplay -q", safeText);
            String[] exec = {"/bin/sh", "-c", cmd};
            Runtime.getRuntime().exec(exec);
        } catch (Exception e) {
            System.err.println("播报失败: " + e.getMessage());
        }
    }
}