// Shader-ul de fragment / Fragment shader  
 #version 330

in vec4 ex_Color;
uniform int codCuloare;
 
out vec4 out_Color;

void main(void)
{
  switch (codCuloare)
  {
	case 0: //returneaza culorile originale
	  out_Color = ex_Color;
	  break;
	case 1: //returneaza culoarea soarelui
		out_Color=vec4 (0.988, 0.898, 0.439, 0.0);
		break;
	case 2: //returneaza culoarea norilor
		out_Color=vec4 (0.866, 0.906, 0.933, 0.0);
		break;
	case 3: //returneaza culoarea corpului pasarilor
		out_Color=vec4 (1, 0.4, 0.0, 0.0);
		break;
	case 4: //returneaza culoarea ciocului pasarilor
		out_Color=vec4 (0.847, 0.733, 0.255, 0.0);
		break;
	case 5: //returneaza culoarea ochiului pasarilor
		out_Color=vec4 (0.0, 0.0, 0.0, 0.0);
		break;
	default:
		break;
  };
}
 