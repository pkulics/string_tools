#include <iostream>
#include <sstream>
#include <string>
#include "boost/regex.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string/regex.hpp"
#include <iconv.h>
#include <cfloat>
#include <sstream>
#include "string_tool.hpp"

using namespace std;
using namespace boost;

static const string date_regex_str = "^(公元(前)?\\d+年)|(\\d+年\\d+月\\d+日)|(\\d+年)|(\\d+月)|(\\d+日)|(\\d+年\\d+月)|(\\d+月\\d+日)$";
static const boost::regex date_regex(date_regex_str.c_str());
static const boost::regex kuohao_regex("(\\(.*?\\))");
static const boost::regex quotation_regex("\"(.*?)\"");
static const boost::regex shuming_regex("《(.*?)》");

static const int BUFFER_SIZE = 40960;

namespace qa_short {

    const unordered_map<string, string> ChineseEnglishPucRespond::chinese_english_puc_respond = ChineseEnglishPucRespond::create_map();
    const unordered_set<string> EnglishPunc::english_punc = EnglishPunc::create_set();
    const unordered_set<string> SentenceSpliter::sentence_spliter = SentenceSpliter::create_set();

    unsigned char ToHex(unsigned char x)   
    {   
        return  x > 9 ? x + 55 : x + 48;   
    }  

    unsigned char FromHex(unsigned char x)   
    {   
        unsigned char y;  
        if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;  
        else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;  
        else if (x >= '0' && x <= '9') y = x - '0';  
        else assert(0);  
        return y;  
    }  

    string UrlEncode(const string& str)
    {  
        string strTemp = "";  
        size_t length = str.length();  
        for (size_t i = 0; i < length; i++)  
        {  
            if (isalnum((unsigned char)str[i]) ||   
                    (str[i] == '-') ||  
                    (str[i] == '_') ||   
                    (str[i] == '.') ||   
                    (str[i] == '~'))  
                strTemp += str[i];  
            else if (str[i] == ' ')  
                strTemp += "+";  
            else  
            {  
                strTemp += '%';  
                strTemp += ToHex((unsigned char)str[i] >> 4);  
                strTemp += ToHex((unsigned char)str[i] % 16);  
            }  
        }  
        return strTemp;  
    }  

    // 全角转半角
    string SBC2DBC(const string &sbc)
    {
        const char sbc_high = -93;
        const char sbc_space = -95;
        string dbc = "";
        int len = sbc.length();
        for (int i=0; i<len; ++i)
        {
            if (sbc[i] > 0)    //已经是单字节字符，或者是控制字符
            {
                dbc.append(1, sbc[i]);
            }
            else
            {
                if (sbc[i] == sbc_high)    //全角的英文字母或全角英文符号，如！(A3A1)
                {
                    dbc.append(1, sbc[i+1]&0x7f);
                }
                else if (sbc[i]==sbc_space && sbc[i+1]==sbc_space)    //单独处理空格
                {
                    dbc.append(1, ' ');
                }
                else //针对汉字以及～……等中文符号
                {
                    dbc += sbc.substr(i, 2);
                }
                ++i;
            }
        }
        return dbc;
    }

    // 半角转全角
    string DBC2SBC(const string &dbc)
    {
        const char sbc_high = -93;
        const char sbc_space = -95;
        const string space_str = "\xa1\xa1";
        const string bolang = "\xa1\xab";
        string sbc = "";
        int len = dbc.length();
        for (int i=0; i<len; ++i)
        {
            if (dbc[i] < 0)    //已经是双字节字符，或者是汉字及中文符号
            {
                sbc += dbc.substr(i, 2);
                ++i;
            }
            else if (dbc[i] == ' ')    //单独处理空格
            {
                sbc += space_str;
            }
            else if (dbc[i] == '~')    //单独处理波浪
            {
                sbc += bolang;
            }
            else
            {
                if (dbc[i]>=33 && dbc[i]<=126)//半角的英文字母或半角英文符号
                {
                    sbc.append(1, sbc_high);
                    sbc.append(1, dbc[i]|0x80);
                }
                else
                {
                    sbc.append(1, dbc[i]);//控制字符
                }
            }
        }
        return sbc;
    }

    string UrlDecode(const string& str)  
    {  
        string strTemp = "";  
        size_t length = str.length();  
        for (size_t i = 0; i < length; i++)  
        {  
            if (str[i] == '+') strTemp += ' ';  
            else if (str[i] == '%')  
            {  
                assert(i + 2 < length);  
                unsigned char high = FromHex((unsigned char)str[++i]);  
                unsigned char low = FromHex((unsigned char)str[++i]);  
                strTemp += high*16 + low;  
            }  
            else strTemp += str[i];  
        }  
        return strTemp;  
    }  

    bool IsAllEnglishStr(const string& str)
    {
        for (string::size_type i = 0; i < str.size(); i++)
        {
            if (!isalpha(str[i]))
            {
                return false;
            }
        }
        return true;
    }

    bool SplitUTF8String(const string& str, vector<string>& vecs)
    {
        return SplitUTF8String(str.c_str(), str.length(), vecs);
    }

    bool SplitUTF8String(const char * str, int size, vector<string>& vecs)
    {
        bool is_utf8 = true;
        const char* start = str;
        const char* end = str + size;
        while (start < end)
        {
            if ((unsigned char)*start < 0x80)                  // (10000000): 值小于0x80的为ASCII字符
            {
                vecs.push_back(string(start, 1));
                start++;
            }
            else if ((unsigned char)*start < (0xC0))           // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
            {
                is_utf8 = false;
                break;
            }
            else if ((unsigned char)*start < (0xE0))           // (11100000): 此范围内为2字节UTF-8字符
            {
                if (start >= end - 1)
                    break;
                if ((start[1] & (0xC0)) != 0x80)
                {
                    is_utf8 = false;
                    break;
                }
                vecs.push_back(string(start, 2));
                start += 2;
            }
            else if ((unsigned char)*start < (0xF0))           // (11110000): 此范围内为3字节UTF-8字符
            {
                if (start >= end - 2)
                    break;
                if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
                {
                    is_utf8 = false;
                    break;
                }
                vecs.push_back(string(start, 3));
                start += 3;
            }
            else
            {
                is_utf8 = false;
                break;
            }
        }
        return is_utf8;
    }

    int CountUTF8ChNum(const string& str)
    {
        int ch_num = 0;
        CountUTF8ChNum(str.c_str(), str.length(), ch_num);
        return ch_num;
    }

    bool CountUTF8ChNum(const char * str, int size, int& ch_num)
    {
        bool is_utf8 = true;
        ch_num = 0;
        const char* start = str;
        const char* end = str + size;
        while (start < end)
        {
            if ((unsigned char)*start < 0x80)                  // (10000000): 值小于0x80的为ASCII字符
            {
                ch_num++;
                start++;
            }
            else if ((unsigned char)*start < (0xC0))           // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
            {
                is_utf8 = false;
                break;
            }
            else if ((unsigned char)*start < (0xE0))           // (11100000): 此范围内为2字节UTF-8字符
            {
                if (start >= end - 1)
                    break;
                if ((start[1] & (0xC0)) != 0x80)
                {
                    is_utf8 = false;
                    break;
                }
                ch_num++;
                start += 2;
            }
            else if ((unsigned char)*start < (0xF0))           // (11110000): 此范围内为3字节UTF-8字符
            {
                if (start >= end - 2)
                    break;
                if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
                {
                    is_utf8 = false;
                    break;
                }
                ch_num++;
                start += 3;
            }
            else
            {
                is_utf8 = false;
                break;
            }
        }
        return is_utf8;
    }

    bool ContainNumber(const string& str)
    {
        return ContainNumber(str.c_str(), str.size());
    }

    bool ContainNumber(const char* str, int size)
    {
        const char* start = str;
        const char* end = str + size;
        while (start < end)
        {
            if ((unsigned char)*start < 0x80)                  // (10000000): 值小于0x80的为ASCII字符
            {
                if ((unsigned char)*start >= '0' && (unsigned char)*start <= '9')
                {
                    return true;
                }
                start++;
            }
            else if ((unsigned char)*start < (0xC0))           // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
            {
                break;
            }
            else if ((unsigned char)*start < (0xE0))           // (11100000): 此范围内为2字节UTF-8字符
            {
                if (start >= end - 1)
                    break;
                if ((start[1] & (0xC0)) != 0x80)
                {
                    break;
                }
                start += 2;
            }
            else if ((unsigned char)*start < (0xF0))           // (11110000): 此范围内为3字节UTF-8字符
            {
                if (start >= end - 2)
                    break;
                if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
                {
                    break;
                }
                start += 3;
            }
            else
            {
                break;
            }
        }
        return false;
    }

    bool ContainChinese(const char* str, int size)
    {
        bool contain_chinese = false;
        const char* start = str;
        const char* end = str + size;
        while (start < end)
        {
            if ((unsigned char)*start < 0x80)                  // (10000000): 值小于0x80的为ASCII字符
            {
                start++;
            }
            else if ((unsigned char)*start < (0xC0))           // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
            {
                break;
            }
            else if ((unsigned char)*start < (0xE0))           // (11100000): 此范围内为2字节UTF-8字符
            {
                if (start >= end - 1)
                    break;
                if ((start[1] & (0xC0)) != 0x80)
                {
                    break;
                }
                contain_chinese = true;
                break;
            }
            else if ((unsigned char)*start < (0xF0))           // (11110000): 此范围内为3字节UTF-8字符
            {
                if (start >= end - 2)
                    break;
                if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
                {
                    break;
                }
                contain_chinese = true;
                break;
            }
            else
            {
                break;
            }
        }
        return contain_chinese;
    }

    int CountSubStr(const string& str, const string& sub_str)
    {
        int count = 0;
        int start_pos = 0;
        int find_pos = str.find(sub_str, start_pos);
        while (find_pos != string::npos)
        {
            count += 1;
            start_pos = find_pos + sub_str.length();
            find_pos = str.find(sub_str, start_pos);
        }
        return count;
    }

    bool IsDateTime(const string& str)
    {
        if (str == "周一" || str == "周二" || str == "周三" || str == "周四" || str == "周五" || str == "周六" || str == "周日" || str == "周末" || str == "周天")
            return true;
        if(boost::regex_match(str, date_regex))
            return true;
        return false;
    }

    void GetShumingContent(const string& str, vector<string>& shuming_content)
    {
        boost::match_results<std::string::const_iterator> what;
        boost::match_flag_type flags = boost::match_default;

        string::const_iterator start = str.begin();
        string::const_iterator end = str.end();

        while(regex_search(start, end, what, shuming_regex, flags))   
        {     
            shuming_content.push_back(string(what[1].first, what[1].second));
            start = what[0].second;  
        }
    }

    void GetQuotationContent(const string& str, vector<string>& quotation_content)
    {
        boost::match_results<std::string::const_iterator> what;
        boost::match_flag_type flags = boost::match_default;

        string::const_iterator start = str.begin();
        string::const_iterator end = str.end();

        while(regex_search(start, end, what, quotation_regex, flags))   
        {     
            quotation_content.push_back(string(what[1].first, what[1].second));
            start = what[0].second;  
        }
    }

    string DropKuohaoContent(const string& str)
    {
        return boost::regex_replace(str, kuohao_regex, "");
    }

    int lcs(const string& s1, const string& s2)
    {
        vector<string> vec1;
        SplitUTF8String(s1, vec1);
        vector<string> vec2;
        SplitUTF8String(s2, vec2);

        return lcs(vec1, vec2);
    }

    int EditDistance(vector<string>& vec1, vector<string>& vec2)
    {
        int length1 = vec1.size();
        int length2 = vec2.size();
        int **dp = new int*[length1+1];
        for (int i = 0; i <= length1; ++i)
            dp[i] = new int[length2+1];
        for (int i = 0; i <= length1; ++i)
        {
            for (int j = 0; j <= length2; ++j)
            {
                if (0 == i && 0 == j)
                    dp[i][j] = 0;
                else if (0 == i && j > 0)
                    dp[i][j] = j;
                else if (i > 0 && j == 0)
                    dp[i][j] = i;
                else
                {
                    int cost = vec1[i-1] == vec2[j-1] ? 0 : 1;
                    dp[i][j] = Min3(dp[i-1][j] + 1, dp[i][j-1] + 1, dp[i-1][j-1] + cost);
                }
            }
        }
        int edit_distance = dp[length1][length2];
        for (int i = 0; i <= length1; ++i)
            delete[] dp[i];
        delete[] dp;
        return edit_distance;
    }

    int EditDistance(vector<string>& vec1, vector<string>& vec2, int start, int end)
    {
        if(start < 0 || end > vec1.size() || start > end)
            return -1;
        //int length1 = vec1.size();
        int length1 = end - start;
        int length2 = vec2.size();
        int **dp = new int*[length1+1];
        for (int i = 0; i <= length1; ++i)
            dp[i] = new int[length2+1];
        for (int i = 0; i <= length1; ++i)
        {
            for (int j = 0; j <= length2; ++j)
            {
                if (0 == i && 0 == j)
                    dp[i][j] = 0;
                else if (0 == i && j > 0)
                    dp[i][j] = j;
                else if (i > 0 && j == 0)
                    dp[i][j] = i;
                else
                {
                    //int cost = vec1[i-1] == vec2[j-1] ? 0 : 1;
                    int cost = vec1[start+i-1] == vec2[j-1] ? 0 : 1;
                    int tmp = dp[i-1][j] + 1;
                    if (dp[i][j-1] + 1 < tmp)
                    {
                        tmp = dp[i][j-1] + 1;
                    }
                    if (dp[i-1][j-1] + cost < tmp)
                    {
                        tmp = dp[i-1][j-1] + cost;
                    }
                    dp[i][j] = tmp;
                }
            }
        }
        int edit_distance = dp[length1][length2];
        for (int i = 0; i <= length1; ++i)
            delete[] dp[i];
        delete[] dp;
        return edit_distance;
    }

    int lcs(vector<string>& vec1, vector<string>& vec2)
    {
        int length1 = vec1.size();
        int length2 = vec2.size();
        int **dp = new int*[length1+1];
        for (int i = 0; i <= length1; ++i)
            dp[i] = new int[length2+1];

        for (int i = 0; i <= length1; ++i)
        {
            for (int j = 0; j <= length2; ++j)
            {
                if (0 == i || 0 == j)
                    dp[i][j] = 0;
                else if (vec1[i-1] == vec2[j-1])
                    dp[i][j] = dp[i-1][j-1] + 1;
                else
                    dp[i][j] = Max(dp[i-1][j], dp[i][j-1]);
            }
        }
        int lcs_length = dp[length1][length2];
        for (int i = 0; i <= length1; ++i)
            delete[] dp[i];
        delete[] dp;
        return lcs_length;
    }

    int lcs(const char* s1, const char* s2, int length1, int length2)
    {
        int **dp = new int*[length1+1];
        for (int i = 0; i <= length1; ++i)
            dp[i] = new int[length2+1];
        for (int i = 0; i <= length1; ++i)
        {
            for (int j = 0; j <= length2; ++j)
            {
                if (0 == i || 0 == j)
                    dp[i][j] = 0;
                else if (s1[i-1] == s2[j-1])
                    dp[i][j] = dp[i-1][j-1] + 1;
                else
                    dp[i][j] = Max(dp[i-1][j], dp[i][j-1]);
            }
        }
        int lcs_length = dp[length1][length2];
        for (int i = 0; i <= length1; ++i)
            delete[] dp[i];
        delete[] dp;
        return lcs_length;
    }

    double Max(double a, double b)
    {
        if (a > b)
            return a;
        return b;
    }

    int Max(int a, int b)
    {
        if (a > b)
            return a;
        return b;
    }

    int Min3(int a, int b, int c)
    {
        int d = a < b ? a : b;
        return d < c ? d : c;
    }

    bool IsDoubleEqual(double x, double y)
    {
        return IsEqualZero(x - y);
    }

    bool IsEqualZero(double x)
    {
        if (x > -DBL_MIN && x < DBL_MIN)
            return true;
        return false;
    }

    void SplitString(const string& str, const string& split_str, vector<string>& result, bool remain_empty)
    {
        result.clear();
        if (split_str == "")
        {
            return;
        }
        int pos1 = 0;
        int pos2 = str.find(split_str);
        while (pos2 != string::npos)
        {
            string tmp_str = str.substr(pos1, pos2-pos1);
            if (remain_empty || tmp_str != "")
            {
                result.push_back(tmp_str);
            }
            pos1 = pos2 + split_str.size();
            pos2 = str.find(split_str, pos1);
        }
        if (pos1 != str.length())
        {
            string tmp_str = str.substr(pos1);
            if (remain_empty || tmp_str != "")
            {
                result.push_back(tmp_str);
            }
        }
    }

    bool StringStartsWith(const string& s1, const string& s2)
    {
        return boost::algorithm::starts_with(s1, s2);
    }

    bool StringEndsWith(const string& s1, const string& s2)
    {
        return boost::algorithm::ends_with(s1, s2);
    }

    string TrimString(const string& str)
    {
        return boost::algorithm::trim_copy(str);
    }

    bool ContainsSubString(const string&s1, const string& s2)
    {
        return boost::algorithm::contains(s1, s2);
    }

    string Utf8ToGbkIgnore(const string& in)
    {
        char buff[BUFFER_SIZE];
        size_t outlen = BUFFER_SIZE; 
        int ret = Utf8ToGbkIgnore(in.c_str(), in.size(), buff, outlen);
        string out = buff;
        return out;
    }

    string GbkToUtf8Ignore(const string& in)
    {
        char buff[BUFFER_SIZE];
        size_t outlen = BUFFER_SIZE; 
        int ret = GbkToUtf8Ignore(in.c_str(), in.size(), buff, outlen);
        string out = buff;
        return out;
    }

    string Utf8ToUtf16Ignore(const string& in)
    {
        char buff[BUFFER_SIZE];
        size_t outlen = BUFFER_SIZE; 
        int ret = Utf8ToUtf16Ignore(in.c_str(), in.size(), buff, outlen);
        string out = buff;
        return out;
    }

    string Utf16ToUtf8Ignore(const string& in)
    {
        char buff[BUFFER_SIZE];
        size_t outlen = BUFFER_SIZE; 
        int ret = Utf16ToUtf8Ignore(in.c_str(), in.size(), buff, outlen);
        string out = buff;
        return out;
    }

    string GbkToUtf16Ignore(const string& in)
    {
        char buff[BUFFER_SIZE];
        size_t outlen = BUFFER_SIZE; 
        int ret = GbkToUtf16Ignore(in.c_str(), in.size(), buff, outlen);
        string out = buff;
        return out;
    }

    string Utf16ToGbkIgnore(const string& in)
    {
        char buff[BUFFER_SIZE];
        size_t outlen = BUFFER_SIZE; 
        int ret = Utf16ToGbkIgnore(in.c_str(), in.size(), buff, outlen);
        string out = buff;
        return out;
    }

    int code_convert(const string& from_charset, const string& to_charset,const char *inbuf, size_t inlen,char *outbuf, size_t& outlen)
    {
        iconv_t cd;
        char *inbuf_tmp = (char*)inbuf;
        char *pin = inbuf_tmp;
        char *pout = outbuf;

        cd = iconv_open(to_charset.c_str(),from_charset.c_str());
        if (cd==(iconv_t)-1) {
            iconv_close(cd);
            //cerr<<"[ICONV OPEN ERROR]"<<endl;
            return -1;
        }
        memset(outbuf,0,outlen);
        int ts = iconv(cd, &pin, &inlen, &pout, &outlen);
        if (ts==-1){
            iconv_close(cd);
            //cerr<<"[ICONV ERROR]:"<<ts<<endl;
            return -1;
        }
        iconv_close(cd);
        return 0;
    }

    /* UTF-8 to GBK  */
    int Utf8ToGbkIgnore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen)
    {
        return code_convert("UTF-8//IGNORE","GB18030//IGNORE",inbuf,inlen,outbuf,outlen);
    }

    /* GBK to UTF-8 */
    int GbkToUtf8Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen)
    {
        return code_convert("GB18030//IGNORE", "UTF-8//IGNORE", inbuf, inlen, outbuf, outlen);
    }

    int Utf8ToUtf16Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen)
    {
        return code_convert("UTF-8//IGNORE", "UTF-16//IGNORE", inbuf, inlen, outbuf, outlen);
    }

    int Utf16ToUtf8Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen)
    {
        return code_convert("UTF-16//IGNORE", "UTF-8//IGNORE", inbuf, inlen, outbuf, outlen);
    }

    int GbkToUtf16Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen)
    {
        return code_convert("GB18030//IGNORE", "UTF-16//IGNORE", inbuf, inlen, outbuf, outlen);
    }

    int Utf16ToGbkIgnore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen)
    {
        return code_convert("UTF-16//IGNORE", "GB18030//IGNORE", inbuf, inlen, outbuf, outlen);
    }

    string TransferPunc(const string& input)
    {
        const unordered_map<string, string>& chinese_english_puc_respond = ChineseEnglishPucRespond::chinese_english_puc_respond;
        const unordered_set<string>& english_punc = EnglishPunc::english_punc;
        /*
           unordered_map<string, string> chinese_english_puc_respond;
           unordered_set<string> english_punc;
           */

        vector<string> vecs;
        SplitUTF8String(input, vecs);
        string output = "";
        for (size_t i = 0; i < vecs.size(); ++i)
        {
            //if (ChineseEnglishPucRespond::chinese_english_puc_respond.find(vecs[i]) != ChineseEnglishPucRespond::chinese_english_puc_respond.end())
            if (chinese_english_puc_respond.find(vecs[i]) != chinese_english_puc_respond.end())
                //output += ChineseEnglishPucRespond::chinese_english_puc_respond.find(vecs[i])->second;
                output += chinese_english_puc_respond.find(vecs[i])->second;
            else
                output += vecs[i];
        }
        return output;
    }

    string DropPunc(const string& input)
    {
        const unordered_map<string, string>& chinese_english_puc_respond = ChineseEnglishPucRespond::chinese_english_puc_respond;
        const unordered_set<string>& english_punc = EnglishPunc::english_punc;

        /*
           unordered_map<string, string> chinese_english_puc_respond;
           unordered_set<string> english_punc;
           */

        vector<string> vecs;
        SplitUTF8String(input, vecs);
        string output = "";
        for (size_t i = 0; i < vecs.size(); ++i)
        {
            if (english_punc.find(vecs[i]) != english_punc.end() || chinese_english_puc_respond.find(vecs[i]) != chinese_english_puc_respond.end())
                output += " ";
            else
                output += vecs[i];
        }
        return output;
    }

    void GetSetIntersection(const unordered_set<string>& set1, const unordered_set<string>& set2, unordered_set<string>& intersection)
    {
        for (unordered_set<string>::const_iterator cit = set1.begin(); cit != set1.end(); ++cit)
        {
            if (set2.find(*cit) != set2.end())
            {
                intersection.insert(*cit);
            }
        }
        for (unordered_set<string>::const_iterator cit = set2.begin(); cit != set2.end(); ++cit)
        {
            if (set1.find(*cit) != set1.end())
            {
                intersection.insert(*cit);
            }
        }
    }

    string FormatSearchStringRemainRedMark(const string& input)
    {
        string output = input;
        boost::replace_all(output,"\r\n"," ");
        boost::replace_all(output,"\r"," ");
        boost::replace_all(output,"\n"," ");
        boost::replace_all(output,"\t"," ");
        boost::replace_all(output,"\xe3\x80\x80"," ");
        return output;
    }

    string FormatSearchString(const string& input)
    {
        string output = input;
        boost::replace_all(output,"\r\n"," ");
        boost::replace_all(output,"\r"," ");
        boost::replace_all(output,"\n"," ");
        boost::replace_all(output,"\t"," ");
        boost::replace_all(output,"\xee\x90\x8a","");
        boost::replace_all(output,"\xee\x90\x8b","");
        boost::replace_all(output,"\xee\x94\x9a","");
        boost::replace_all(output,"\xee\x94\x9b","");
        return output;
    }

    bool SplitSentences(const string& input, vector<pair<string, string> >& sentences)
    {
        const unordered_set<string>& sentence_spliter = SentenceSpliter::sentence_spliter;
        SplitSentences(input.c_str(), input.size(), sentence_spliter, sentences);
    }

    bool SplitSentences(const string& input, const unordered_set<string>& sentence_spliter, vector<pair<string, string> >& sentences)
    {
        SplitSentences(input.c_str(), input.size(), sentence_spliter, sentences);
    }

    bool SplitSentences(const char* str, int size, const unordered_set<string>& sentence_spliter, vector<pair<string, string> >& sentences)
    {
        //const unordered_set<string>& sentence_spliter = SentenceSpliter::sentence_spliter;
        bool is_utf8 = true;
        const char* start = str;
        const char* end = str + size;

        sentences.clear();
        string sentence = "";
        while (start < end)
        {
            if ((unsigned char)*start < 0x80)                  // (10000000): 值小于0x80的为ASCII字符
            {
                string cur_s(string(start, 1));
                if (sentence_spliter.find(cur_s) != sentence_spliter.end())
                {
                    sentences.push_back(make_pair(sentence, cur_s));
                    sentence = "";
                }
                else
                {
                    sentence += cur_s;
                }
                start++;
            }
            else if ((unsigned char)*start < (0xC0))           // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
            {
                is_utf8 = false;
                break;
            }
            else if ((unsigned char)*start < (0xE0))           // (11100000): 此范围内为2字节UTF-8字符
            {
                if (start >= end - 1)
                    break;
                if ((start[1] & (0xC0)) != 0x80)
                {
                    is_utf8 = false;
                    break;
                }

                string cur_s(string(start, 2));
                if (sentence_spliter.find(cur_s) != sentence_spliter.end())
                {
                    sentences.push_back(make_pair(sentence, cur_s));
                    sentence = "";
                }
                else
                {
                    sentence += cur_s;
                }

                start += 2;
            }
            else if ((unsigned char)*start < (0xF0))           // (11110000): 此范围内为3字节UTF-8字符
            {
                if (start >= end - 2)
                    break;
                if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
                {
                    is_utf8 = false;
                    break;
                }
                string cur_s(string(start, 3));
                if (sentence_spliter.find(cur_s) != sentence_spliter.end())
                {
                    sentences.push_back(make_pair(sentence, cur_s));
                    sentence = "";
                }
                else
                {
                    sentence += cur_s;
                }
                start += 3;
            }
            else
            {
                is_utf8 = false;
                break;
            }
        }
        if (sentence != "")
        {
            sentences.push_back(make_pair(sentence, ""));
        }
        return is_utf8;
    }

    string FormatSupportText(const string& input)
    {
        string output = input;
        boost::replace_all(output, "……", "…");
        output = GbkToUtf8Ignore(SBC2DBC(Utf8ToGbkIgnore(output)));
        boost::replace_all(output, "\t", "");
        boost::replace_all(output, "\r\n", "");
        return output;
    }

    string FormatStringRemainSpace(const string& input)
    {
        string output = input;
        boost::replace_all(output, "……", "。");
        boost::replace_all(output, "…", "。");
        boost::replace_all(output, "．．．", "。");
        boost::replace_all(output, "...", "。");
        output = GbkToUtf8Ignore(SBC2DBC(Utf8ToGbkIgnore(output)));
        output = TransferPunc(output);
        boost::replace_all(output, "...", "。");
        //boost::replace_all(output, "-", "");
        boost::replace_all(output, "\t", "");
        boost::replace_all(output, "\r\n", "");
        boost::algorithm::to_lower(output);
        return output;
    }

    string FormatString(const string& input)
    {
        string output = input;
        boost::replace_all(output, "……", "。");
        boost::replace_all(output, "…", "。");
        boost::replace_all(output, "．．．", "。");
        boost::replace_all(output, "...", "。");
        output = GbkToUtf8Ignore(SBC2DBC(Utf8ToGbkIgnore(output)));
        output = TransferPunc(output);
        boost::replace_all(output, "...", "。");
        //boost::replace_all(output, "-", "");
        boost::replace_all(output, "\t", "");
        boost::replace_all(output, "\r\n", "");
        boost::replace_all(output, " ", "");
        boost::algorithm::to_lower(output);
        return output;
    }

    string ReplaceAll(const string& input, const string& from_s, const string& to_s)
    {
        string output = boost::replace_all_copy(input, from_s, to_s);
        return output;
    }

    string ReplaceFirst(const string& input, const string& from_s, const string& to_s)
    {
        string output = boost::replace_first_copy(input, from_s, to_s);
        return output;
    }

    string ReplaceLast(const string& input, const string& from_s, const string& to_s)
    {
        string output = boost::replace_last_copy(input, from_s, to_s);
        return output;
    }

    string LowerString(const string& input)
    {
        string output = boost::algorithm::to_lower_copy(input);
        return output;
    }

    string JoinStrings(const vector<string>& input, const string& join_str)
    {
        string output = boost::algorithm::join(input, join_str);
        return output;
    }

    string JoinStrings(const unordered_set<string>& input, const string& join_str)
    {
        vector<string> input_vec(input.begin(), input.end());
        return JoinStrings(input_vec, join_str);
    }
    bool isNum(string& str)
    {
        stringstream sin(str);
        double d;
        string rest;
        if(!(sin>>d))
            return false;
        if( !sin.eof() )
        {
            sin >> rest;
            //cout << "rest:" << rest << endl;
            if(rest != "%")
                return false;
        }
        return true;
    }

    bool IsChineseCharacter(const string& token)
    {
        if (CountUTF8ChNum(token) == 1 && ContainChinese(token.c_str(), token.length()))
            return true;
        return false;
    }

    string GetDomainFromUrl(const string& url)
    {
        string domain = url;
        domain = ReplaceFirst(domain, "http://", "");
        domain = ReplaceFirst(domain, "https://", "");
        int find_pos = domain.find("/");
        if (find_pos != string::npos)
        {
            domain = domain.substr(0, find_pos);
        }
        return domain;
    }

    string ToString(int input)
    {
        stringstream ss;
        ss << input;
        return ss.str();
    }

    string ToString(double input)
    {
        stringstream ss;
        ss << input;
        return ss.str();
    }

    string GetLongestCommonPrefix(const vector<string>& strs)
    {
        if (strs.empty() || strs[0].size() == 0)
            return "";
        if (strs.size() == 1)
            return strs[0];
        string common_prefix = "";
        vector<vector<string> > split_strs;
        for (int i = 0; i < strs.size(); ++i)
        {
            vector<string> split_str;
            SplitUTF8String(strs[i], split_str);
            split_strs.push_back(split_str);
        }
        for (int i = 0; i < split_strs[0].size(); ++i)
        {
            for (int j = 1; j < split_strs.size(); ++j)
            {
                if (i >= split_strs[j].size() || split_strs[j][i] != split_strs[0][i])
                {
                    return common_prefix;
                }
            }
            common_prefix += split_strs[0][i];
        }

        /*
           for (int i = 0; i < strs[0].size(); ++i)
           {
           for (int j = 1; j < strs.size(); ++j)
           {
           if (i > strs[j].size() || strs[j][i] != strs[0][i])
           {
           return strs[0].substr(0, i);
           }
           }
           }
           */
        return strs[0];
    }

    string GetLongestCommonPostfix(const vector<string>& strs)
    {
        if (strs.empty() || strs[0].size() == 0)
            return "";
        if (strs.size() == 1)
            return strs[0];
        string common_postfix = "";
        vector<vector<string> > split_strs;
        for (int i = 0; i < strs.size(); ++i)
        {
            vector<string> split_str;
            SplitUTF8String(strs[i], split_str);
            split_strs.push_back(split_str);
        }
        for (int i = 0; i < split_strs[0].size(); ++i)
        {
            for (int j = 1; j < split_strs.size(); ++j)
            {
                if (i >= split_strs[j].size() || split_strs[j][split_strs[j].size() - 1 - i] != split_strs[0][split_strs[0].size() - 1 - i])
                {
                    return common_postfix;
                }
            }
            common_postfix = split_strs[0][split_strs[0].size() - 1 - i] + common_postfix;
        }
        /*
           for (int i = 0; i < strs[0].size(); ++i)
           {
           for (int j = 1; j < strs.size(); ++j)
           {
           if (i >= strs[j].size() || strs[j][strs[j].size() - 1 - i] != strs[0][strs[0].size() - 1 - i])
           {
           return strs[0].substr(strs[0].size() - i, i);
           }
           }
           }
           */
        return strs[0];
    }

    bool IsChoiceText(const string& text)
    {
        if (text.find("A.") != string::npos && text.find("B.") != string::npos ||
                text.find("B.") != string::npos && text.find("C.") != string::npos ||
                text.find("C.") != string::npos && text.find("D.") != string::npos ||
                text.find("A、") != string::npos && text.find("B、") != string::npos ||
                text.find("B、") != string::npos && text.find("C、") != string::npos ||
                text.find("C、") != string::npos && text.find("D、") != string::npos ||
                text.find("A:") != string::npos && text.find("B:") != string::npos ||
                text.find("B:") != string::npos && text.find("C:") != string::npos ||
                text.find("C:") != string::npos && text.find("D:") != string::npos ||
                text.find("a.") != string::npos && text.find("b.") != string::npos ||
                text.find("b.") != string::npos && text.find("c.") != string::npos ||
                text.find("c.") != string::npos && text.find("d.") != string::npos ||
                text.find("a、") != string::npos && text.find("b、") != string::npos ||
                text.find("b、") != string::npos && text.find("c、") != string::npos ||
                text.find("c、") != string::npos && text.find("d、") != string::npos ||
                text.find("a:") != string::npos && text.find("b:") != string::npos ||
                text.find("b:") != string::npos && text.find("c:") != string::npos ||
                text.find("c:") != string::npos && text.find("d:") != string::npos )
                /*
                text.find("1.") != string::npos && text.find("2.") != string::npos ||
                text.find("2.") != string::npos && text.find("3.") != string::npos ||
                text.find("1、") != string::npos && text.find("2、") != string::npos ||
                text.find("2、") != string::npos && text.find("3、") != string::npos)
                */
            return true;
        return false;
    }

} // end of namespace qa_short
