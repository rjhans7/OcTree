intersecta (nodo n, (void *)(punto, punto) ecuacion) {
	punto eje = ecuacion ((nodo.x_min + nodo.x_max) / 2);
	if (n.x_min < eje.x < n.x_max and n.y_min < eje.y < n.y_max) {
		return true;
	}
	return false;
}

cut_vertical ((void *)(punto, punto) ecuacion, nodo root) {
	if (intersecta (p1, p2, ecuacion, nodo)) {
		if (nodo es hoja) {
			pintar (nodo.color);
			return;
		}
		
		for (i = 0 to nodo.children)
			cut_vertical (ecuacion, nodo.children[i]);
	}
	return;
}
