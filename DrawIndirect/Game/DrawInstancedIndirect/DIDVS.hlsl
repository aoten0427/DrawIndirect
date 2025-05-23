#include"DID.hlsli"





// ���_�V�F�[�_�[�̏C��
PSPNTInput main(VSPNTInstanceInput input)
{
    PSPNTInput result;
	//���_�̈ʒu��ϊ�
    float4 pos = float4(input.Pos.xyz, 1.0f);
	//���[���h�ϊ�
    pos = mul(pos, input.mat);
	//�r���[�ϊ�
    pos = mul(pos, View);
	//�ˉe�ϊ�
    pos = mul(pos, Projection);
	//�s�N�Z���V�F�[�_�ɓn���ϐ��ɐݒ�
    result.position = pos;
	//���C�e�B���O
    result.norm = mul(input.Normal, (float3x3) input.mat);
    result.norm = normalize(result.norm);
	//�e�N�X�`��UV
    result.tex = input.Tex;
    return result;
}