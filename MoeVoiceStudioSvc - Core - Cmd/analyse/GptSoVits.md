# GptSoVits��Ҫ��Ϊ��������

### VQ���ں�ΪKMeans���ࣩ
- Train����ѵ������Ƶ��ssl[^1]����KMeans���࣬��ȡ���ľ������Ĺ���һ��Embedding��CodeBook.embed��
- Infer��ʹ��Indices��ȡ�������ģ�CodeBook.embed���е�Ԫ�أ�����һ��ssl[^1]����

��SoVits��KMeans/Index�������ƣ�ֻ����SoVits�ľ�����ʹ��ʱ��ʹ�������HuBert��CodeBook�в��������������ǰK�ĵ���Ȩƽ������GptSoVits����ʹ��һ��ARѭ��Ԥ�������HuBert��CodeBook�е��±֮꣬��ʹ�ø��±��ȡCodeBook�ж�ӦԪ��

---

### AR(GPT)
- Inputs:
	- text_seq�������ı��������е�����ID����Symbols�����е��±꣩
	- text_bert�������ı���Bert
	- ref_seq���ο��ı��������е�����ID����Symbols�����е��±꣩
	- ref_bert���ο��ı���Bert
	- ref_ssl���ο���Ƶ��ssl[^1]
- OutPuts:
	- codes�����뵽VQ��Indices�����ڻ�ȡssl[^1]�ľ�������

��Gpt���ƣ�ʹ��һ��ARѭ����ͨ�������ı���������ϢԤ��һ����Ӧ���У�������ֹΪEOS��������Ӧ����Ϊѵ������Ƶ�����ľ���������CodeBook�е��±֮꣬����CodeBook�л�ȡ��Ӧ��Ԫ�أ��൱��SoVits�е�Hubert��

---

### SoVits
- Inputs:
	- codes�����뵽VQ��Indices�����ڻ�ȡssl[^1]�ľ�������
	- text_seq�������ı��������е�����ID����Symbols�����е��±꣩
	- ref_audio���ο���Ƶ��ѵ��������Ƶ��

��SoVits�Ƚϣ����е�codesʵ�����൱��SoVits��Hubert��ʹ��

---

[^1]: ssl��ʵ������Ƶ��Hubert����SoVits��Hubertһ��