pkgname=pacboy
_name=pacboy
pkgver=1.0.0
pkgrel=1
pkgdesc="A simple library manager to handle custom C libraries, to copy them as needed"
arch=("x86_64" "aarch75")
url="https://git.greensky.tf/Greensky/pacboy"
license=("unknown")
depends=()
makedepends=('git' 'gcc' 'make')
optdepends=()
source=("git+${url}#commit=cbcc085d5409616c4849d22a5f37d75b8a123f9c")
options=("!debug")
sha256sums=("SKIP")

build() {
	cd "$srcdir/$_name"

	make cleanbuild
}
package() {
	cd "$srcdir/$_name"

	# Binary
	install -Dm755 bin/main.uwu "$pkgdir/usr/bin/pacboy"

	# Man page
	install -Dm644 docs/pacboy.1 "$pkgdir/usr/share/man/man1/pacboy.1"
}
