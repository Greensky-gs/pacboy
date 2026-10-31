pkgname=pacboy
_name=pacboy
pkgver=1.0.2
pkgrel=2
pkgdesc="A simple library manager to handle custom C libraries, to copy them as needed"
arch=("x86_64" "aarch64")
url="https://git.greensky.tf/Greensky/pacboy"
license=("unknown")
depends=()
makedepends=('git' 'gcc' 'make')
optdepends=()
source=("git+${url}#commit=964a81c7fd67d6512fdec72dd91cf4c504c0af6b")
options=("!debug")
sha256sums=("SKIP")

build() {
  cd "$srcdir/$_name"

  make cleanbuild PROG_VERSION="$pkgver" DEFAULT_MIRROR_URL="https://git.greensky.tf/Greensky/paclibs-database"
}
package() {
  cd "$srcdir/$_name"

  # Binary
  install -Dm755 bin/main.uwu "$pkgdir/usr/bin/pacboy"

  # Man page
  install -Dm644 docs/pacboy.1 "$pkgdir/usr/share/man/man1/pacboy.1"
}
