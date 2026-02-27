import time
import datetime

def get_com_password():
    """
    根据当前日期和时间生成复杂密码
    
    密码结构（9个字符）：
    - 第一和最后位置：月份中的日期（如果日期为奇数则交换位置）
    - 第二和倒数第二位置：小时+12（如果小时+12为奇数则交换位置）
    - 中间四个位置：由日期和小时数字转换得到的ASCII字母（数字+65）
    - 中间位置：基于其他字符求和的特殊字符
    
    返回：一个9字符的密码
    """
    # 获取当前时间
    now = datetime.datetime.now()
    day = now.day
    hour = now.hour
    
    # 将日期和小时转换为字符串，如果需要则添加前导零
    day_str = f"{day:02d}"
    hour_plus_str = f"{hour + 12:02d}"
    
    # 创建密码列表（9个字符）
    password = [''] * 9
    
    # 将日期数字放在第一和最后位置（如果日期为奇数则交换）
    if day % 2 == 0:
        # 偶数日期：正常顺序
        password[0] = day_str[0]
        password[8] = day_str[1]
    else:
        # 奇数日期：交换顺序
        password[0] = day_str[1]
        password[8] = day_str[0]
    
    # 将小时+12的数字放在第二和倒数第二位置（如果小时+12为奇数则交换）
    hour_plus = hour + 12
    if hour_plus % 2 == 0:
        # 偶数小时+12：正常顺序
        password[1] = hour_plus_str[0]
        password[7] = hour_plus_str[1]
    else:
        # 奇数小时+12：交换顺序
        password[1] = hour_plus_str[1]
        password[7] = hour_plus_str[0]
    
    # 将日期和小时数字转换为ASCII字母（数字 + 65）
    password[2] = chr(int(day_str[0]) + 65)  # 第一个日期数字转换为字母
    password[3] = chr(int(day_str[1]) + 65)  # 第二个日期数字转换为字母
    password[5] = chr(int(hour_plus_str[0]) + 65)  # 第一个小时+12数字转换为字母
    password[6] = chr(int(hour_plus_str[1]) + 65)  # 第二个小时+12数字转换为字母
    
    # 计算8个字符的总和，用于选择特殊字符
    sum_val = 0
    for i in range(9):
        if i != 4:  # 跳过中间位置（尚未设置）
            char = password[i]
            if 'A' <= char <= 'Z':
                sum_val += ord(char) - ord('A') + 10  # 对于字母，使用值10-35
            else:
                sum_val += int(char)  # 对于数字，使用数值
    
    # 根据总和%10选择特殊字符
    special_chars = "!@#$%^&*()"
    password[4] = special_chars[sum_val % 10]
    
    return ''.join(password)

if __name__ == "__main__":
    # 生成并打印密码
    password = get_com_password()
    print(f"生成的密码: {password}")
    
    # 显示当前时间信息（用于验证）
    now = datetime.datetime.now()
    print(f"当前日期: {now.day:02d}")
    print(f"当前小时: {now.hour:02d}")
    print(f"小时+12: {now.hour + 12:02d}")
