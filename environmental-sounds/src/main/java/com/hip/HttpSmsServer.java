// src/com/hip/HttpSmsServer.java
package com.hip;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.concurrent.Executors;

public class HttpSmsServer {

    public static void start(int port) {
        try {
            HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
            server.createContext("/send-sms", new SmsHandler());
            server.setExecutor(Executors.newFixedThreadPool(5)); // 小线程池
            server.start();
            System.out.println("短信 API 已启动: http://localhost:" + port + "/send-sms");
        } catch (IOException e) {
            System.err.println("无法启动短信 API 服务: " + e.getMessage());
        }
    }

    static class SmsHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\":\"仅支持 GET\"}");
                return;
            }

            try {
             SmsUtil.sendVerificationCode();
                    sendResponse(exchange, 200, "{\"status\":\"success\",\"message\":\"短信已发送\"}");
            } catch (Exception e) {
                e.printStackTrace();
                sendResponse(exchange, 500, "{\"status\":\"error\",\"message\":\"内部异常\"}");
            }
        }

        private void sendResponse(HttpExchange exchange, int code, String json) throws IOException {
            byte[] bytes = json.getBytes("UTF-8");
            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
            exchange.sendResponseHeaders(code, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }
}