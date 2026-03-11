"""Spain."""

# standard
from typing import Dict

# local
from validators.utils import validator


def _nif_nie_validation(value: str, number_by_letter: Dict[str, str]):
    """Validate if the doi is a NIF or a NIE."""
    if len(value) != 9:
        return False
    value = value.upper()
    table = "TRWAGMYFPDXBNJZSQVHLCKE"
    # If it is not a DNI, convert the first
    # letter to the corresponding digit
    numbers = number_by_letter.get(value[0], value[0]) + value[1:8]
    # doi[8] is control
    return numbers.isdigit() and value[8] == table[int(numbers) % 23]


@validator
def es_cif(value: str, /):
    """Validate a Spanish CIF.

    Each company in Spain prior to 2008 had a distinct CIF and has been
    discontinued. For more information see [wikipedia.org/cif][1].

    The new replacement is to use NIF for absolutely everything. The issue is
    that there are "types" of NIFs now: company, person [citizen or resident]
    all distinguished by the first character of the DOI. For this reason we
    will continue to call CIFs NIFs, that are used for companies.

    This validator is based on [generadordni.es][2].

    [1]: https://es.wikipedia.org/wiki/C%C3%B3digo_de_identificaci%C3%B3n_fiscal
    [2]: https://generadordni.es/

    Examples:
        >>> es_cif('B25162520')
        True
        >>> es_cif('B25162529')
        ValidationError(func=es_cif, args={'value': 'B25162529'})

    Args:
        value:
            DOI string which is to be validated.

    Returns:
        (Literal[True]): If `value` is a valid DOI string.
        (ValidationError): If `value` is an invalid DOI string.
    """
    if not value or len(value) != 9:
        return False
    value = value.upper()
    table = "JABCDEFGHI"
    first_chr = value[0]
    doi_body = value[1:8]
    control = value[8]
    if not doi_body.isdigit():
        return False
    res = (
        10
        - sum(
            # Multiply each positionally even doi
            # digit by 2 and sum it all together
            sum(map(int, str(int(char) * 2))) if index % 2 == 0 else int(char)
            for index, char in enumerate(doi_body)
        )
        % 10
    ) % 10
    if first_chr in "ABEH":  # Number type
        return str(res) == control
    if first_chr in "PSQW":  # Letter type
        return table[res] == control
    return control in {str(res), table[res]} if first_chr in "CDFGJNRUV" else False


@validator
def es_nif(value: str, /):
    """Validate a Spanish NIF.

    Each entity, be it person or company in Spain has a distinct NIF. Since
    we've designated CIF to be a company NIF, this NIF is only for person.
    For more information see [wikipedia.org/nif][1]. This validator
    is based on [generadordni.es][2].

    [1]: https://es.wikipedia.org/wiki/N%C3%BAmero_de_identificaci%C3%B3n_fiscal
    [2]: https://generadordni.es/

    Examples:
        >>> es_nif('26643189N')
        True
        >>> es_nif('26643189X')
        ValidationError(func=es_nif, args={'value': '26643189X'})

    Args:
        value:
            DOI string which is to be validated.

    Returns:
        (Literal[True]): If `value` is a valid DOI string.
        (ValidationError): If `value` is an invalid DOI string.
    """
    number_by_letter = {"L": "0", "M": "0", "K": "0"}
    return _nif_nie_validation(value, number_by_letter)


@validator
def es_nie(value: str, /):
    """Validate a Spanish NIE.

    The NIE is a tax identification number in Spain, known in Spanish
    as the NIE, or more formally the Número de identidad de extranjero.
    For more information see [wikipedia.org/nie][1]. This validator
    is based on [generadordni.es][2].

    [1]: https://es.wikipedia.org/wiki/N%C3%BAmero_de_identidad_de_extranjero
    [2]: https://generadordni.es/

    Examples:
        >>> es_nie('X0095892M')
        True
        >>> es_nie('X0095892X')
        ValidationError(func=es_nie, args={'value': 'X0095892X'})

    Args:
        value:
            DOI string which is to be validated.

    Returns:
        (Literal[True]): If `value` is a valid DOI string.
        (ValidationError): If `value` is an invalid DOI string.
    """
    number_by_letter = {"X": "0", "Y": "1", "Z": "2"}
    # NIE must must start with X Y or Z
    if value and value[0] in number_by_letter:
        return _nif_nie_validation(value, number_by_letter)
    return False


@validator
def es_doi(value: str, /):
    """Validate a Spanish DOI.

    A DOI in spain is all NIF / CIF / NIE / DNI -- a digital ID.
    For more information see [wikipedia.org/doi][1]. This validator
    is based on [generadordni.es][2].

    [1]: https://es.wikipedia.org/wiki/Identificador_de_objeto_digital
    [2]: https://generadordni.es/

    Examples:
        >>> es_doi('X0095892M')
        True
        >>> es_doi('X0095892X')
        ValidationError(func=es_doi, args={'value': 'X0095892X'})

    Args:
        value:
            DOI string which is to be validated.

    Returns:
        (Literal[True]): If `value` is a valid DOI string.
        (ValidationError): If `value` is an invalid DOI string.
    """
    return es_nie(value) or es_nif(value) or es_cif(value)
