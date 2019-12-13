#ifndef SERVERS_YIZHANDAODI_STRING_TOOL_H
#define SERVERS_YIZHANDAODI_STRING_TOOL_H

#include <string>
#include <vector>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
//#include <unordered_map>
//#include <unordered_set>
#include <assert.h>

using std::string;
using std::pair;
using std::vector;
using std::tr1::unordered_map;
using std::tr1::unordered_set;
//using std::unordered_map;
//using std::unordered_set;

namespace qa_short {

struct ChineseEnglishPucRespond
{
    static unordered_map<string, string> create_map()
    {
        unordered_map<string, string> chinese_english_puc_respond;
        chinese_english_puc_respond["）"] = ")";
        chinese_english_puc_respond["；"] = ";";
        chinese_english_puc_respond["—"] = "-";
        chinese_english_puc_respond["！"] = "!";
        chinese_english_puc_respond["’"] = "'";
        chinese_english_puc_respond["‘"] = "'";
        chinese_english_puc_respond["："] = ":";
        chinese_english_puc_respond["】"] = "]";
        chinese_english_puc_respond["”"] = "\"";
        chinese_english_puc_respond["“"] = "\"";
        chinese_english_puc_respond["？"] = "?";
        chinese_english_puc_respond["【"] = "[";
        //chinese_english_puc_respond["…"] = "...";
        //chinese_english_puc_respond["..."] = "…";
        chinese_english_puc_respond["（"] = "(";
        //chinese_english_puc_respond["、"] = ",";
        chinese_english_puc_respond["，"] = ",";
        chinese_english_puc_respond["～"] = "~";
        chinese_english_puc_respond["•"] = "";
        chinese_english_puc_respond["·"] = "";
        chinese_english_puc_respond["．"] = ".";
        //chinese_english_puc_respond["﹑"] = "-";
        chinese_english_puc_respond["﹑"] = "、";
        return chinese_english_puc_respond;
    }
    static const unordered_map<string, string> chinese_english_puc_respond;
};

struct EnglishPunc
{
    static unordered_set<string> create_set()
    {
        unordered_set<string> english_punc;
        english_punc.insert("!");
        english_punc.insert("#");
        english_punc.insert("\"");
        english_punc.insert("%");
        english_punc.insert("'");
        english_punc.insert("&");
        english_punc.insert(")");
        english_punc.insert("(");
        english_punc.insert("+");
        english_punc.insert("*");
        english_punc.insert("-");
        english_punc.insert(",");
        english_punc.insert("/");
        english_punc.insert(".");
        english_punc.insert(";");
        english_punc.insert(":");
        english_punc.insert("=");
        english_punc.insert("<");
        english_punc.insert("?");
        english_punc.insert(">");
        english_punc.insert("@");
        return english_punc;
    }
    static const unordered_set<string> english_punc;
};

struct SentenceSpliter
{
    static unordered_set<string> create_set()
    {
        unordered_set<string> sentence_spliter;

        sentence_spliter.insert(",");
        sentence_spliter.insert("，");

        sentence_spliter.insert("。");

        sentence_spliter.insert("?");
        sentence_spliter.insert("？");

        sentence_spliter.insert(";");
        sentence_spliter.insert("；");

        sentence_spliter.insert("!");
        sentence_spliter.insert("！");

        sentence_spliter.insert("…");

        //sentence_spliter.insert(" ");

        return sentence_spliter;
    }
    static const unordered_set<string> sentence_spliter;
};

unsigned char ToHex(unsigned char x);
unsigned char FromHex(unsigned char x);

string UrlEncode(const string& str);
string UrlDecode(const string& str);
string SBC2DBC(const string& sbc);
string DBC2SBC(const string& dbc);
bool IsAllEnglishStr(const string& str);
bool SplitUTF8String(const string& str, vector<string>& vecs);
bool SplitUTF8String(const char* str, int size, vector<string>& vecs);
int CountUTF8ChNum(const string& str);
bool CountUTF8ChNum(const char* str, int size, int& ch_num);
bool isNum(string& str);
bool ContainChinese(const char* str, int size);
bool ContainNumber(const string& str);
bool ContainNumber(const char* str, int size);
int CountSubStr(const string& str, const string& sub_str);
bool IsDateTime(const string& str);
void GetShumingContent(const string& str, vector<string>& shuming_content);
void GetQuotationContent(const string& str, vector<string>& quotation_content);
string DropKuohaoContent(const string& str);
int lcs(const string& s1, const string& s2);
int lcs(vector<string>& vec1, vector<string>& vec2);
int EditDistance(vector<string>& vec1, vector<string>& vec2);
int EditDistance(vector<string>& vec1, vector<string>& vec2, int start, int end);
int lcs(const char* s1, const char* s2, int length1, int length2);
double Max(double a, double b);
int Max(int a, int b);
int Min3(int a, int b, int c);
bool IsEqualZero(double x);
bool IsDoubleEqual(double x, double y);
//void SplitString(const char* str, const char* split_str, vector<string>& result);
void SplitString(const string& str, const string& split_str, vector<string>& result, bool remain_empty=true);
bool StringStartsWith(const string& s1, const string& s2);
bool StringEndsWith(const string& s1, const string& s2);
string TrimString(const string& str);
bool ContainsSubString(const string&s1, const string& s2);
string TransferPunc(const string& input);
string DropPunc(const string& input);
bool SplitSentences(const string& input, vector<pair<string, string> >& sentences);
bool SplitSentences(const string& input, const unordered_set<string>& sentence_spliter, vector<pair<string, string> >& sentences);
bool SplitSentences(const char* str, int size, const unordered_set<string>& sentence_spliter, vector<pair<string, string> >& sentences);
void GetSetIntersection(const unordered_set<string>& set1, const unordered_set<string>& set2, unordered_set<string>& intersection);
string FormatSearchStringRemainRedMark(const string& input);
string FormatSearchString(const string& input);
string FormatStringRemainSpace(const string& input);
string FormatSupportText(const string& input);
string FormatString(const string& input);
string ReplaceAll(const string& input, const string& from_s, const string& to_s);
string ReplaceFirst(const string& input, const string& from_s, const string& to_s);
string ReplaceLast(const string& input, const string& from_s, const string& to_s);
string LowerString(const string& input);
string JoinStrings(const vector<string>& input, const string& join_str);
string JoinStrings(const unordered_set<string>& input, const string& join_str);
bool IsChineseCharacter(const string& token);
string GetDomainFromUrl(const string& url);
string Utf8ToGbkIgnore(const string& in);
string GbkToUtf8Ignore(const string& in);
string Utf8ToUtf16Ignore(const string& in);
string Utf16ToUtf8Ignore(const string& in);
string GbkToUtf16Ignore(const string& in);
string Utf16ToGbkIgnore(const string& in);
int code_convert(const string& from_charset,const string& to_charset,const char *inbuf, size_t inlen,char *outbuf, size_t& outlen);
int Utf8ToGbkIgnore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen);
int GbkToUtf8Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen);
int Utf8ToUtf16Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen);
int Utf16ToUtf8Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen);
int GbkToUtf16Ignore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen);
int Utf16ToGbkIgnore(const char *inbuf, size_t inlen, char *outbuf, size_t& outlen);
bool IsChoiceText(const string& text);

string ToString(int input);
string ToString(double input);
string GetLongestCommonPrefix(const vector<string>& strs);
string GetLongestCommonPostfix(const vector<string>& strs);

} // end of namespace qa_short
#endif
