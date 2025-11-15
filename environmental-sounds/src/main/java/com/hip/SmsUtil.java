package com.hip;

import com.aliyun.teaopenapi.models.Config;
import com.aliyun.dysmsapi20170525.Client;
import com.aliyun.dysmsapi20170525.models.SendSmsRequest;
import com.aliyun.dysmsapi20170525.models.SendSmsResponse;

public class SmsUtil {

    private static final String ACCESS_KEY_ID = "XXX";
    private static final String ACCESS_KEY_SECRET = "XXX";

    // 使用阿里云官方测试签名 + 测试模板（仅用于验证码）
    private static final String SIGN_NAME = "阿里云短信测试";
    private static final String TEMPLATE_CODE = "SMS_154950909";

    // 接收短信的手机号(没有营业执照，所以只能用自己的手机号测试)
    private static final String PHONE_NUMBER = "XXX";

    private static Client client;

    static {
        try {
            Config config = new Config()
                    .setAccessKeyId(ACCESS_KEY_ID)
                    .setAccessKeySecret(ACCESS_KEY_SECRET);
            config.endpoint = "dysmsapi.aliyuncs.com";
            client = new Client(config);
        } catch (Exception e) {
            System.err.println(" 短信客户端初始化失败: " + e.getMessage());
        }
    }

    /**
     * 发送验证码短信（固定为 111111）
     *
     * @return
     */
    public static boolean sendVerificationCode() {
        if (client == null) {
            System.err.println("短信客户端未初始化，跳过发送");
            return false;
        }

        try {
            String templateParam = "{\"code\":\"111111\"}";

            SendSmsRequest request = new SendSmsRequest()
                    .setPhoneNumbers(PHONE_NUMBER)
                    .setSignName(SIGN_NAME)
                    .setTemplateCode(TEMPLATE_CODE)
                    .setTemplateParam(templateParam);

            SendSmsResponse response = client.sendSms(request);

            if ("OK".equals(response.getBody().getCode())) {
                System.out.println("短信验证码 111111 已发送！BizId: " + response.getBody().getBizId());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }
}