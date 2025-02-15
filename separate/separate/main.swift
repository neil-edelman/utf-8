import Foundation

var count = 0
while let line = readLine() {
	let split = try? NSRegularExpression(pattern: #"\w+"#)
	/* The NSString is internally in utf-16. */
	let matches = split!.matches(in: line, range: NSRange(location: 0, length: line.utf16.count))
	count += matches.count
	/*for match in matches {
		print((line as NSString).substring(with: match.range))
	}*/
}
print("Count", count)
